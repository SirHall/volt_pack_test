#include "volt/net/net_con.hpp"
#include "volt/event/global_event.hpp"
#include "volt/net/event_types/e_closed_con.hpp"
#include "volt/net/event_types/e_new_con.hpp"
#include "volt/net/network.hpp"
#include "volt/net/volt_net_defs.hpp"

#include <assert.h>
#include <iostream>
#include <string>

using namespace volt::net;

#pragma region Global Functions

std::unique_ptr<net_con>
    net_con::new_connection(tcp::socket connected_socket,
                            std::shared_ptr<boost::asio::io_context> io_context,
                            con_id connection_id)
{
    return std::unique_ptr<net_con>(
        new net_con(std::move(connected_socket), io_context, connection_id));
}

void net_con::close_con()
{
    if (!this->con_open.load())
        return;
    this->con_open.store(false);
    this->sock.cancel();
    this->sock.close();
    this->con_closed_callback(this->get_con_id());
}

#pragma endregion

#pragma region Member Functions

bool net_con::get_next_byte(net_word &next_byte)
{
    // std::cout << (msg_size == 0) << " " << (current_index >= msg_size) << " "
    //           << !this->is_open() << std::endl;
    if ((msg_size == 0) || (current_index >= msg_size) || !this->is_open())
    {
        current_index = 0;
        return false;
    }
    next_byte = buff.at(current_index);
    current_index++;
    return true;
}

net_con::net_con(tcp::socket                              connected_socket,
                 std::shared_ptr<boost::asio::io_context> io_context,
                 con_id                                   connection_id)
    : sock(std::move(connected_socket)), con_open(true), io(io_context),
      send_strand(*io_context), recv_strand(*io_context), id(connection_id),
      is_reading(false), recv_msg(msg_pool::get_message())
{
    this->recv_msg->resize(0);
    this->buff.resize(2048);
}

net_con::~net_con()
{
    this->close_con();
    std::cout << "NetCon closed" << std::endl;
    // using namespace volt::event;
    // global_event<e_closed_con>::call_event(e_closed_con(this->get_con_id()));
}

void net_con::schedule_read_handler()
{
    this->sock.async_read_some(
        boost::asio::buffer(buff),
        boost::asio::bind_executor(
            recv_strand,
            boost::bind(&net_con::handle_read, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred)));
}

void net_con::handle_read(const boost::system::error_code &err,
                          std::size_t                      bytes_transferred)
{
    // if (this->recv_msg)
    this->msg_size = bytes_transferred;
    if (!err)
    {
        // if (!this->recv_msg)
        // {
        //     this->recv_msg = msg_pool::get_message();
        //     this->recv_msg->resize(0);
        // }

        net_word next_byte = 0;

        while (this->is_open() && get_next_byte(next_byte))
        {
            if ((next_byte != escape_val) && !this->next_is_escape)
            {
                // This is a normal byte
                this->recv_msg->push_back(next_byte);
            }
            else if ((next_byte == escape_val) && !this->next_is_escape)
            {
                // This byte signals that this next character is an escape
                // sequence
                this->next_is_escape = true;
                continue;
            }
            else if (this->next_is_escape)
            {
                // This is the next character in the escape sequence
                switch (next_byte)
                {
                    case msg_origi_char:
                        // This is just the original character
                        this->recv_msg->push_back(escape_val);
                        break;
                    case msg_end_escaped:
                        // This is the end of the message
                        this->new_msg_callback(this->get_con_id(),
                                               std::move(recv_msg));
                        this->recv_msg = msg_pool::get_message();
                        this->recv_msg->resize(0);
                        break;
                    default:
                        // TODO: Throw some 'unrecognized escape sequence' error
                        std::cerr << "Received unrecognized escape sequence: "
                                  << (int)next_byte << std::endl;
                        break;
                }
                // We finished the escape sequence
                this->next_is_escape = false;
            }
        }

        if (this->is_open())
        {
            this->schedule_read_handler();
        }
    }
    else if (err == boost::asio::error::eof)
    {
        std::cout << "Connection closed" << std::endl;
        this->close_con();
    }
    else
    {
        std::cout << "Error with reading" << std::endl;
        this->close_con();
    }
}

void net_con::send_msg(message_ptr m)
{
    boost::asio::async_write(
        sock, boost::asio::buffer(*m),
        boost::asio::bind_executor(
            send_strand,
            boost::bind(&net_con::handle_write, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred, m)));
}

void net_con::handle_write(const boost::system::error_code &err,
                           std::size_t bytes_transferred, message_ptr msg)
{
    if (err)
    {
        std::cout << "Error occurred while trying to send message" << std::endl;
        // this->close_con();
    }

    if (bytes_transferred < msg->size())
    {
        throw std::runtime_error("bytes_transferred < msg->size()");
    }

    // We have a copy of the message shared pointer so only when the message is
    // sent will we free it. In turn, when all the messages have been sent, the
    // message will be cleaned.
}

bool net_con::is_open() { return this->con_open.load(); }

void net_con::set_closed_callback(std::function<void(con_id)> callback)
{
    this->con_closed_callback = callback;
}

void net_con::set_new_msg_callback(
    std::function<void(con_id, message_ptr)> callback)
{
    this->new_msg_callback = callback;
    if (!this->is_reading.load())
    {
        this->schedule_read_handler();
        this->is_reading.store(true);
    }
}

#pragma endregion
