require 'socket'

# 连接服务器函数
def connect_to_server
  TCPSocket.new('localhost', 12345)
end

# 发送命令函数
def send_command(socket, source, command, payload = "")
  message = "#{source} #{command} #{payload}".strip
  socket.puts(message)
  socket.gets.chomp
end

# 测试插入功能
def test_put(socket)
  response = send_command(socket, "CLIENT", "PUT", "key1 value1")
  puts "PUT: #{response}"

  response = send_command(socket, "CLIENT", "PUT", "key2 value2")
  puts "PUT: #{response}"

  response = send_command(socket, "CLIENT", "PUT", "key3 value3")
  puts "PUT: #{response}"
end

# 测试获取功能
def test_get(socket)
  response = send_command(socket, "CLIENT", "GET", "key1")
  puts "GET key1: #{response}"

  response = send_command(socket, "CLIENT", "GET", "key2")
  puts "GET key2: #{response}"

  response = send_command(socket, "CLIENT", "GET", "key3")
  puts "GET key3: #{response}"

  response = send_command(socket, "CLIENT", "GET", "key4")  # 不存在的键
  puts "GET key4: #{response}"
end

# 测试删除功能
def test_delete(socket)
  response = send_command(socket, "CLIENT", "DELETE", "key1")
  puts "DELETE key1: #{response}"

  response = send_command(socket, "CLIENT", "DELETE", "key2")
  puts "DELETE key2: #{response}"

  response = send_command(socket, "CLIENT", "DELETE", "key3")
  puts "DELETE key3: #{response}"

  response = send_command(socket, "CLIENT", "DELETE", "key4")  # 不存在的键
  puts "DELETE key4: #{response}"
end

# 测试QUIT命令
def test_quit(socket)
  response = send_command(socket, "CLIENT", "QUIT")
  puts "QUIT: #{response}"
end

# 测试SELECT命令
def test_select(socket)
  response = send_command(socket, "CLIENT", "SELECT")
  puts "SELECT: #{response}"
end

# 测试LRU功能
def test_lru(socket)
  # 设置缓存大小为5
  (1..6).each do |i|
    send_command(socket, "CLIENT", "PUT", "key#{i} value#{i}")
    sleep(1)  # 等待1秒
  end

  # 获取被替换的键，应返回"NOT FOUND"
  response = send_command(socket, "CLIENT", "GET", "key1")
  puts "GET key1 (after LRU): #{response}"
end

# 主测试函数
def main
  # 连接到服务器
  socket = connect_to_server

  # 测试插入功能
  puts "Testing PUT command..."
  test_put(socket)
  puts

  # 测试获取功能
  puts "Testing GET command..."
  test_get(socket)
  puts

  # 测试删除功能
  puts "Testing DELETE command..."
  test_delete(socket)
  puts
  
  # 测试LRU功能
  puts "Testing LRU functionality..."
  test_lru(socket)
  puts

  # 测试SELECT命令
  puts "Testing SELECT command..."
  test_select(socket)
  puts

  # 测试QUIT命令
  puts "Testing QUIT command..."
  test_quit(socket)
  puts

  # 关闭连接
  socket.close
end

# 运行测试
main
