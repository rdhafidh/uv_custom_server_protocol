#include <processor.h>
#include <uv.h>
#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <iostream>
#include <vector>

#define DEFAULT_BACKLOG 128

uv_loop_t *loop;
struct sockaddr_in addr;

struct maindata {
  QByteArray mainconfig;
  uv_tcp_t server;
  uv_signal_t signal;
  std::size_t maxreadbuffer = 0;
  uv_loop_t *loop = nullptr;
};

class MybufferIo {
 public:
  MybufferIo() {}
  ~MybufferIo() {
    buffer.clear();
    printf("\n~MybufferIo called\n");
  }

  std::string getmbfrombytes(size_t n) {
    size_t mb = 1024 * 1024;
    std::string myret;

    double numret = n / mb;
    myret = QString::number(numret, 'f', 2).toStdString();
    std::cout << "myret" << myret.c_str();
    if (n < mb) {
      myret += " kb";
    } else {
      myret += " mb";
    }
    return myret;
  }

  void printbuf(const char *base, ssize_t nread) {
    ++times;
    std::cout << "times" << times;
    std::size_t current = buffer.length() + nread;
    if (current > rawbuffer->maxreadbuffer) {
      std::cout
          << "\nbuffer.length ()+nread)>rawbuffer->maxreadbuffer current: "
          << current << "\n";
      if (is_readclienttimer_running) {
        std::cout << "\nstopping timer first\n";
        uv_timer_stop(&readclienttimer);
        std::cout << "\ntimer reading client data stopped\n";
        this->is_readclienttimer_running = false;
      }
      closeclient((uv_handle_t *)&client, false);
      return;
    } else {
      buffer.append(base, nread);
      if (times == 1) {
        // try to validate buffer length
        std::vector<std::string> ch = spliting(buffer, "#");
        if (ch.size() != 2) {
          std::cout << "ch.size ()!=2";
          closeclient((uv_handle_t *)&client, false);
          return;
        }
        bool ok = false;
        int length = QByteArray::fromStdString(ch.at(0)).toInt(&ok);
        if (!ok || length > rawbuffer->maxreadbuffer || length < 1) {
          std::cout << "\n!ok || length>rawbuffer->maxreadbuffer|| length<1\n";
          std::cout << "\nlength: " << length
                    << " maxreadbuffer: " << rawbuffer->maxreadbuffer;
          closeclient((uv_handle_t *)&client, false);
          return;
        }
        actuallengthask = length;
        buffer = ch.at(1);
        ch.clear();
      }

      if (buffer.size() == actuallengthask) {
        // handle jika read data timer masih running
        if (is_readclienttimer_running) {
          std::cout << "\nstopping timer first at " << times << "\n";
          uv_timer_stop(&readclienttimer);
          std::cout << "\ntimer reading client data stopped\n";
          this->is_readclienttimer_running = false;
        }
        balas(Processor::render(&buffer, this), true);
      } else {
        // client bisa saja bohong kurang dari actuallengthask
        // nah ini yang susah jika client ga close maka dia hangs
        // pertama harus detek kalau memang bener2 dia udah ga ngirimi data
        // sampe 20 detik lamanya harus selesai semua terkirim kalo tidak
        // force close koneksi
        if (!is_readclienttimer_running && times == 1) {
          readclienttimer.data = (void *)this;
          uv_timer_init(rawbuffer->loop, &readclienttimer);
          uv_timer_start(
              &readclienttimer,
              [](uv_timer_t *handle) {
                MybufferIo *io = static_cast<MybufferIo *>(handle->data);
                printf("\nio->readclienttimer_times: %d\n",
                       io->readclienttimer_times);
                if (io->readclienttimer_times == 20) {
                  if (io->is_readclienttimer_running) {
                    printf(
                        "\nreached timeout reading data from client, closing "
                        "timer\n");
                    uv_timer_stop(handle);
                    printf("\ntimer closed \n");
                    io->is_readclienttimer_running = false;
                  }
                  io->is_readclienttimer_running = false;
                  MybufferIo::closeclient((uv_handle_t *)&io->client, false);
                  return;
                }
                ++io->readclienttimer_times;
              },
              0, 1000);
          is_readclienttimer_running = true;
        }
      }
      if (buffer.size() > 1) {
        std::cout << "\nlast buffer char " << buffer.at(buffer.size() - 1)
                  << " actual length: " << actuallengthask
                  << " buffer size: " << buffer.size() << "\n";
      }
    }
  }

  std::vector<std::string> spliting(std::string data, std::string token) {
	  /*
	  TODO provide better implementation. 
	  */
    std::vector<std::string> output;
    std::size_t pos = std::string::npos;   
    do {
      pos = data.find(token);
      output.push_back(data.substr(0, pos));
      if (std::string::npos != pos) data = data.substr(pos + token.size());
    } while (std::string::npos != pos);
    return output;
  }
  void balas(const std::string &rp, bool asktostopreading = false) {
    printf("\nexecuting balas..\n");
    if (asktostopreading) {
      uv_read_stop(reinterpret_cast<uv_stream_t *>(&this->client));
    }
    // detect lagi apakah client handle masih belum close
    if (!uv_is_closing((uv_handle_t *)&client)) {
      this->singlewritecontext.data = (void *)this;
      uv_buf_t mybuf = uv_buf_init((char *)rp.c_str(), rp.size());
      uv_write(&this->singlewritecontext, (uv_stream_t *)&client,
               (const uv_buf_t *)&mybuf, 1, [](uv_write_t *req, int status) {
                 if (status < 0) {
                   printf("write failed!");
                 }
                 printf("\ncloseing client connection..\n");
                 MybufferIo::closeclient((uv_handle_t *)req->handle);
               });
    } else {
      // sekarang muncul pertanyaan kenapa ini bisa terjadi ?
      // apakah client sudah force close?
      printf("\nfailed balas client data deleting all handles neknu :(\n");
      if (is_readclienttimer_running) {
        std::cout << "\nstopping timer first\n";
        uv_timer_stop(&readclienttimer);
        std::cout << "\ntimer reading client data stopped\n";
        this->is_readclienttimer_running = false;
      }
    }
  }

  static void closeclient(uv_handle_t *req, bool iswrite = true) {
    if (iswrite) {
      if (!uv_is_closing((uv_handle_t *)req)) {
        uv_close((uv_handle_t *)req, [](uv_handle_t *handle) {
          MybufferIo *io = static_cast<MybufferIo *>(handle->data);
          printf("\nattempting deleting client handle..\n");
          delete io;
          io = nullptr;
          printf("\n client connection closed horrey....\n");
        });
      } else {
        MybufferIo *io = static_cast<MybufferIo *>(req->data);
        printf("\nattempting deleting client handle..\n");
        delete io;
        io = nullptr;
      }
    } else {
      if (!uv_is_closing((uv_handle_t *)req)) {
        uv_close((uv_handle_t *)req, [](uv_handle_t *handle) {
          MybufferIo *io = static_cast<MybufferIo *>(handle->data);
          printf("\nattempting deleting client handle..\n");
          delete io;
          io = nullptr;
          printf("\n client connection closed horrey....\n");
        });
      } else {
        MybufferIo *io = static_cast<MybufferIo *>(req->data);
        printf("\nattempting deleting client handle..\n");
        delete io;
        io = nullptr;
      }
    }
  }

  uv_tcp_t client;
  std::string buffer;
  bool bad = true;

  qint64 times = 0;
  // whether echo_read is already called
  bool recvbuf = false;
  std::size_t actuallengthask = 0;
  // timer timout state, default true
  bool timerrunning = false;
  signed timertimes = 0;
  uv_timer_t mytimer;

  uv_timer_t readclienttimer;
  bool is_readclienttimer_running = false;
  signed readclienttimer_times = 0;

  uv_write_t singlewritecontext;
  maindata *rawbuffer = nullptr;
};

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = new char[suggested_size];
  buf->len = suggested_size;
  printf("\nsuggested_size %zi\n", suggested_size);
}

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
  MybufferIo *io = static_cast<MybufferIo *>(client->data);
  // bisajadi ini called 1 menit kemudan
  io->recvbuf = true;
  if (io->times == 0) {
    if (io->timerrunning) {
      uv_timer_stop(&io->mytimer);
      io->timerrunning = false;
      printf("\ntimer  stopped..\n");
    }
    printf("\ntimeout becaouse echo_read called!\n");
  }
  if (nread < 0) {
    io->bad = true;
    if (io->is_readclienttimer_running) {
      printf("\ntimer read is running, stoppng that timer..");
      uv_timer_stop(&io->readclienttimer);
      io->is_readclienttimer_running = false;
      printf("\ntimer stopped\n");
    }
    if (!uv_is_closing((uv_handle_t *)client)) {
      uv_close((uv_handle_t *)client, [](uv_handle_t *handle) {
        MybufferIo *io = static_cast<MybufferIo *>(handle->data);
        delete io;
        io = nullptr;
      });
    } else {
      delete io;
      io = nullptr;
    }
    if (nread != UV_EOF) fprintf(stderr, "Read error %s\n", uv_err_name(nread));
  } else if (nread >= 0) {
    io->bad = false;
    io->printbuf(static_cast<const char *>(buf->base), nread);
  } else if (nread == 0) {
    uv_read_stop((uv_stream_t *)client);
    if (!uv_is_closing((uv_handle_t *)client)) {
      uv_close((uv_handle_t *)client, [](uv_handle_t *handle) {
        MybufferIo *io = static_cast<MybufferIo *>(handle->data);
        delete io;
        io = nullptr;
      });
    } else {
      delete io;
      io = nullptr;
    }
    printf("\nwarning nread 0 byts...");
  }
  printf("echo_read called\n");
  if (nread == UV_EOF) {
    // disini normal client close
    printf("\nreached EOF!!");
    if (io->is_readclienttimer_running) {
      printf("\ntimer read is running, stoppng that timer..");
      uv_timer_stop(&io->readclienttimer);
      printf("\ntimer stopped\n");
      io->is_readclienttimer_running = false;
    }
  }
  if (buf->base) {
    delete buf->base;
  }
}

void on_new_connection(uv_stream_t *server, int status) {
  if (status < 0) {
    fprintf(stderr, "New connection error %s\n", uv_strerror(status));
    // error!
    return;
  }
  MybufferIo *io = new MybufferIo;
  maindata *configdata = static_cast<maindata *>(server->data);
  io->rawbuffer = configdata;
  io->client.data = (void *)io;
  uv_tcp_init(loop, &io->client);

  if (uv_accept(server, (uv_stream_t *)&io->client) == 0) {
    int ret = uv_is_readable((const uv_stream_t *)&io->client);
    if (ret == 0) {
      uv_close((uv_handle_t *)&io->client, [](uv_handle_t *handle) {
        MybufferIo *io = static_cast<MybufferIo *>(handle->data);
        delete io;
      });
      return;
    }
    io->mytimer.data = (void *)io;
    uv_timer_init(loop, &io->mytimer);
    ret = uv_read_start((uv_stream_t *)&io->client, alloc_buffer, echo_read);
    if (ret != 0) fprintf(stderr, "uv_read_start error %s\n", uv_err_name(ret));
    uv_timer_start(
        &io->mytimer,
        [](uv_timer_t *handle) {
          MybufferIo *io = static_cast<MybufferIo *>(handle->data);
          // hangs disini, cek sudah berapa kali hang?
          if (!io->recvbuf && io->timertimes == 3) {
            printf("\nasking to terminate client connection..");
            printf("\nis closing %d\n",
                   uv_is_closing((uv_handle_t *)&io->client));
            uv_timer_stop(handle);
            uv_read_stop(reinterpret_cast<uv_stream_t *>(&io->client));
            if (!uv_is_closing((uv_handle_t *)&io->client)) {
              uv_close((uv_handle_t *)&io->client, [](uv_handle_t *handle) {
                MybufferIo *io = static_cast<MybufferIo *>(handle->data);
                delete io;
                io = nullptr;
              });
            } else {
              printf("\ndeleting unused mem...");
              delete io;
              printf("\ndeleted");
              io = nullptr;
            }
            return;
          }
          io->timertimes++;
        },
        0, 1000);
    io->timerrunning = true;
  } else {
    uv_close((uv_handle_t *)&io->client, [](uv_handle_t *handle) {
      MybufferIo *io = static_cast<MybufferIo *>(handle->data);
      delete io;
      io = nullptr;
    });
  }
}

void signalcb(uv_signal_t *handle, int) {
  maindata *data = static_cast<maindata *>(handle->data);
  uv_signal_stop(handle);
  if (!uv_is_closing((uv_handle_t *)&data->server)) {
    printf("\nclosing tcp server handle\n");
    uv_close((uv_handle_t *)&data->server, [](uv_handle_t *hnd) {
      maindata *data = static_cast<maindata *>(hnd->data);
      uv_loop_t *myloop = data->loop;
      delete data;
      uv_stop(myloop);
      uv_loop_close(myloop);
    });
  } else {
    uv_loop_t *myloop = data->loop;
    delete data;
    data = nullptr;
    uv_stop(myloop);
    uv_loop_close(myloop);
  }
}

int main(int argc, char *argv[]) {
  QCoreApplication a(argc, argv);
  // todo free
  loop = uv_default_loop();
  qputenv("UV_THREADPOOL_SIZE",
          QString::number(QThread::idealThreadCount() + 1).toUtf8());
  // todo free
  maindata *data = new maindata;
  data->loop = loop;
  data->signal.data = (void *)data;
  uv_signal_init(loop, &data->signal);
  uv_signal_start(&data->signal, signalcb, SIGINT);
  /*
	  TODO provide better implementation from
	  either user provided settings config or file
  */
  data->maxreadbuffer = 1400000;

   /*
	  TODO provide opsional either tcp socket or pipe socket over file 
  */
  uv_tcp_init(loop, &data->server); 
  uv_ip4_addr("0.0.0.0", 8000, &addr);

  data->server.data = (void *)data;
  uv_tcp_bind(&data->server, (const struct sockaddr *)&addr, 0);
  int r = uv_listen((uv_stream_t *)&data->server, DEFAULT_BACKLOG,
                    on_new_connection);
  if (r) {
    fprintf(stderr, "Listen error %s\n", uv_strerror(r));
    return 1;
  }
  uv_run(loop, UV_RUN_DEFAULT);
  printf("\ndeleting event loop\n");
  return 0;
}
