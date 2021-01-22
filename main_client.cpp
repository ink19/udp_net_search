#include <boost/asio.hpp>
#include <iostream>
#include <vector>
#include <boost/array.hpp>
#include <string>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>

struct CommonConfig {
  int port;
  std::string ping_msg;
  int cli_port;
  int timer;
};

class udp_client {
public:
  udp_client(boost::asio::io_context &io, CommonConfig &config):
    m_io(io),
    m_config(config),
    m_timer(io, boost::asio::chrono::seconds(config.timer)),
    m_socket(io),
    m_broadcast_endpoint(boost::asio::ip::address_v4::broadcast(), config.port)
  {
    m_socket.open(boost::asio::ip::udp::v4());
    m_socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    m_socket.set_option(boost::asio::socket_base::broadcast(true));
    timer_out_handle();
    // set_timer();
  }

  void set_timer() {
    m_timer.expires_after(boost::asio::chrono::seconds(m_config.timer));
    m_timer.async_wait(boost::bind(&udp_client::timer_out_handle, this));
  }

  void timer_out_handle() {
    m_socket.send_to(
      boost::asio::buffer(m_config.ping_msg),
      m_broadcast_endpoint
    );
    set_timer();
  }
private:
  boost::asio::steady_timer m_timer;
  boost::asio::ip::udp::socket m_socket;
  boost::asio::io_service &m_io;
  boost::asio::ip::udp::endpoint m_broadcast_endpoint;
  CommonConfig &m_config;
};

class udp_server {
public:
  udp_server(boost::asio::io_context &io, CommonConfig& config) : 
  m_io(io),
  m_udp_socket(io, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), config.cli_port)),
  m_config(config)
  {
    start_recive();
  }

  void start_recive() {
    m_udp_socket.async_receive_from(
      boost::asio::buffer(m_recv_buff),
      remote_point,
      boost::bind(
        &udp_server::recive_handle,
        this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred
      )
    );
  }

  void recive_handle(const boost::system::error_code& error, std::size_t recv_size) {
    m_recv_buff[recv_size] = 0;
    std::cout << "Recv:" << m_recv_buff.data() << " From:" << remote_point << std::endl;
    start_recive();
  }

  void send_handle(boost::shared_ptr<std::string>&, const boost::system::error_code&, std::size_t) {

  }

private:
  boost::asio::io_context &m_io;
  boost::asio::ip::udp::socket m_udp_socket;
  boost::asio::ip::udp::endpoint remote_point;
  boost::array<char, 1024> m_recv_buff;
  CommonConfig &m_config;
};



int main(int argc, char *argv[]) {
  boost::program_options::options_description cmd_parser("PingPong Serv");
  cmd_parser.add_options()("port,p", boost::program_options::value<int>()->default_value(10534)->value_name("n"), "监听端口");
  cmd_parser.add_options()("cli_port,c", boost::program_options::value<int>()->default_value(10533)->value_name("n"), "客户端监听端口");
  cmd_parser.add_options()("timer,t", boost::program_options::value<int>()->default_value(10)->value_name("sec"), "广播频率时间");
  cmd_parser.add_options()("ping_msg,m", boost::program_options::value<std::string>()->default_value("Hello")->value_name("msg"), "监听字段");
  cmd_parser.add_options()("help,h", "帮助信息");

  boost::program_options::variables_map var_map;
  try {
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, cmd_parser), var_map);
  }  catch (std::exception &e) {
    std::cout << e.what() << std::endl;
    std::cout << cmd_parser;
    return -1;
  }

  if (var_map.count("help") > 0) {
    std::cout << cmd_parser;
    return 0;
  }

  CommonConfig config;

  config.port = var_map["port"].as<int>();
  config.ping_msg = var_map["ping_msg"].as<std::string>();
  config.cli_port = var_map["cli_port"].as<int>();
  config.timer = var_map["timer"].as<int>();
  boost::asio::io_context io;

  udp_server udp_(io, config);
  udp_client udp_cl(io, config);
  io.run();
  return 0;
}
