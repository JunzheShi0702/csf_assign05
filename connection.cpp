#include <sstream>
#include <cctype>
#include <cassert>
#include "csapp.h"
#include "message.h"
#include "connection.h"

Connection::Connection()
    : m_fd(-1), m_last_result(SUCCESS)
{
}

Connection::Connection(int fd)
    : m_fd(fd), m_last_result(SUCCESS)
{
  // TODO: call rio_readinitb to initialize the rio_t object
  rio_readinitb(&m_fdbuf, m_fd);
}

void Connection::connect(const std::string &hostname, int port)
{
  // TODO: call open_clientfd to connect to the server
  std::string port_str = std::to_string(port);
  m_fd = open_clientfd(hostname.c_str(), port_str.c_str());
  // TODO: call rio_readinitb to initialize the rio_t object
  if (m_fd >= 0)
  {
    rio_readinitb(&m_fdbuf, m_fd);
  }
}

Connection::~Connection()
{
  // TODO: close the socket if it is open
  if (is_open())
  {
    close();
  }
}

bool Connection::is_open() const
{
  // TODO: return true if the connection is open
  return m_fd >= 0;
}

void Connection::close()
{
  // TODO: close the connection if it is open
  if (is_open())
  {
    Close(m_fd);
    m_fd = -1;
  }
}

bool Connection::send(const Message &msg)
{
  // TODO: send a message
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  if (!is_open())
  {
    m_last_result = EOF_OR_ERROR;
    return false;
  }
  std::string formatted_msg = msg.tag + ":" + msg.data + "\n";

  // Check length of message
  if (formatted_msg.length() > Message::MAX_LEN)
  {
    m_last_result = INVALID_MSG;
    return false;
  }

  // Send Message
  ssize_t byte_msg = rio_writen(m_fd, formatted_msg.c_str(), formatted_msg.length());

  // Check successfully send all content
  if (byte_msg != (ssize_t)formatted_msg.length())
  {
    m_last_result = EOF_OR_ERROR;
    return false;
  }

  m_last_result = SUCCESS;
  return true;
}

bool Connection::receive(Message &msg)
{
  // TODO: receive a message, storing its tag and data in msg
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  if (!is_open())
  {
    m_last_result = EOF_OR_ERROR;
    return false;
  }

  // Read one line from the connection to get the message sent
  char msg_buffer[Message::MAX_LEN + 1];
  ssize_t byte_msg_r = rio_readlineb(&m_fdbuf, msg_buffer, Message::MAX_LEN);

  // Check content read successfully
  if (byte_msg_r <= 0)
  {
    m_last_result = EOF_OR_ERROR;
    return false;
  }

  // Extracting message
  // Convert to string and remove the trailing \n
  std::string line(msg_buffer);
  if (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
  {
    line.pop_back();
    if (!line.empty() && line.back() == '\r')
    {
      line.pop_back();
    }
  }

  // Find the colon separator for body and tag extraction
  size_t colon_position = line.find(':');
  if (colon_position >= line.length())
  {
    m_last_result = INVALID_MSG;
    return false;
  }

  // Extract
  msg.tag = line.substr(0, colon_position);
  msg.data = line.substr(colon_position + 1);

  m_last_result = SUCCESS;
  return true;
}
