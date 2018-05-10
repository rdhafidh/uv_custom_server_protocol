#include "processor.h"
#include <flatbuffers/flatbuffers.h>
#include <packet_generated.h>
#include <QByteArray>

Processor::Processor() {}

Processor::~Processor() {}

std::string Processor::render(std::string *in, MybufferIo *io) {
  *in = QByteArray::fromBase64(QByteArray::fromStdString(*in)).toStdString();
  flatbuffers::Verifier ver((const uint8_t *)in->c_str(), in->size());
  if (!PacketHeader::VerifyCommandBuffer(ver)) {
    return std::string("malformed data fmt sore galat bos :(");
  }
  const PacketHeader::Command *cmd =
      PacketHeader::GetCommand((const void *)in->data());
  if (!cmd || !cmd->data()) {
    return std::string("decode  failed bos :( ");
  }
  //the rule of thumb is that do not take a long work to do this 
  //because client might get read timeout socket
  std::cout << "\nfound len bytes " << cmd->data()->size ()<< " float id: " << cmd->id();
  return std::string("horey!!");
}
