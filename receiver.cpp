#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv)
{
  if (argc != 5)
  {
    std::cerr << "Usage: ./receiver [server_address] [port] [username] [room]\n";
    return 1;
  }

  std::string server_hostname = argv[1];
  int server_port = std::stoi(argv[2]);
  std::string username = argv[3];
  std::string room_name = argv[4];

  Connection conn;

  // TODO: connect to server
  try
  {
    conn.connect(server_hostname, server_port);
  }
  catch (const std::exception &e)
  {
    std::cerr << "Failed to connect to server\n";
    return 1;
  }

  if (!conn.is_open())
  {
    std::cerr << "Failed to connect to server\n";
    return 1;
  }

  // TODO: send rlogin and join messages (expect a response from
  //       the server for each one)

  // Send rlogin message
  Message rlogin_msg(TAG_RLOGIN, username); // initialize a login message
  if (!conn.send(rlogin_msg))
  {
    std::cerr << "Failed to send rlogin message\n";
    return 1;
  }

  Message response; // store the response from sending message
  if (!conn.receive(response))
  {
    std::cerr << "Failed to receive rlogin message\n";
    return 1;
  }

  if (response.tag == TAG_ERR)
  {
    std::cerr << response.data << std::endl;
    return 1;
  }

  // Send join message
  Message join_msg(TAG_JOIN, room_name);
  if (!conn.send(join_msg))
  {
    std::cerr << "Failed to send join message\n";
    return 1;
  }

  // Wait for response to join
  if (!conn.receive(response))
  {
    std::cerr << "Failed to receive join response\n";
    return 1;
  }

  if (response.tag == TAG_ERR)
  {
    std::cerr << response.data << std::endl;
    return 1;
  }

  if (response.tag == TAG_ERR)
  {
    std::cerr << response.data << std::endl;
    return 1;
  }

  // TODO: loop waiting for messages from server
  //       (which should be tagged with TAG_DELIVERY)
  while (true)
  {
    Message incoming_msg;
    if (!conn.receive(incoming_msg))
    {
      std::cerr << "Failed to receive message from server\n";
      return 1;
    }

    // Validate the message here
    if (incoming_msg.tag == TAG_DELIVERY)
    {
      // Format check and message parsing
      size_t first_colon = incoming_msg.data.find(":");
      if (first_colon > incoming_msg.data.length())
      {
        std::cerr << "Invalid message format received\n";
        continue; // skip to next iteration
      }

      size_t second_colon = incoming_msg.data.find(":", first_colon + 1);
      if (second_colon > incoming_msg.data.length())
      {
        std::cerr << "Invalid message format received\n";
        continue; // skip to next iteration
      }

      // Extract sender and message text
      std::string sender = incoming_msg.data.substr(first_colon + 1, second_colon - first_colon - 1);
      std::string message_text = incoming_msg.data.substr(second_colon + 1);

      // Display message in required format: [sender]: message_text
      std::cout << sender << ": " << message_text << std::endl;
    }
  }
  return 0;
}
