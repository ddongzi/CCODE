#!/usr/bin/ruby
require 'rspec'

describe 'database' do
  def run_script(commands)
    raw_output = nil
    IO.popen("./db my.db", "r+") do |pipe|
      commands.each do |command|
        pipe.puts command
      end

      pipe.close_write
      raw_output = pipe.read  # 使用 read 读取所有输出
    end
    raw_output.split("\n")
  end

  it 'inserts and retrieves a row' do
    skip "Skipping this test"
    result = run_script([
      "insert 1 user1 person1@example.com",
      "select",
      ".exit",
    ])
    expect(result).to match_array([
      "db > Executed.",
      "db > (1, user1, person1@example.com)",
      "Executed.",
      "db > ",
    ])
  end
   # 最大1400行
  it 'prints error message when table is full' do
    skip "Skipping this test"
    script = (1..1401).map do |i|
      "insert #{i} user#{i} person#{i}@example.com"
    end
    script << ".exit"
    result = run_script(script)
    expect(result[-2]).to eq('db > Error: Table full.')
  end

  it 'allows inserting strings that are the maximum length' do
    skip "Skipping this test"
    long_username = "a"*32
    long_email = "a"*255
    script = [
      "insert 1 #{long_username} #{long_email}",
      "select",
      ".exit",
    ]
    result = run_script(script)
    expect(result).to match_array([
      "db > Executed.",
      "db > (1, #{long_username}, #{long_email})",
      "Executed.",
      "db > ",
    ])
  end
  # 字段溢出
  it 'prints error message if strings are too long' do
    skip "Skipping this test"
    long_username = "a"*33
    long_email = "a"*256
    script = [
      "insert 1 #{long_username} #{long_email}",
      "select",
      ".exit",
    ]
    result = run_script(script)
    expect(result).to match_array([
      "db > String is too long.",
      "db > Executed.",
      "db > ",
    ])
  end

  it 'prints an error message if id is negative' do
    skip "Skipping this test"
    script = [
      "insert -1 cstack foo@bar.com",
      "select",
      ".exit",
    ]
    result = run_script(script)
    expect(result).to match_array([
      "db > ID must be positive.",
      "db > Executed.",
      "db > ",
    ])
  end

  it 'prints constants' do
    script = [
      ".constants",
      ".exit"
    ]
    result = run_script(script)
    puts result
    expect(result).to match_array([
      "db > Constants:",
      "ROW_SIZE: 293",
      "COMMON_NODE_HEADER_SIZE: 6",
      "LEAF_NODE_HEADER_SIZE: 10",
      "LEAF_NODE_CELL_SIZE: 297",
      "LEAF_NODE_SPACE_FOR_CELLS: 4086",
      "LEAF_NODE_MAX_CELLS: 13",
      "db > ",
    ])

  end


  it 'allows printing out the structure of a one-node btree' do
    skip "Skipping this test"
    script = [3, 1, 2].map do |i|
      "insert #{i} user#{i} person#{i}@example.com"
    end
    script << ".btree"
    script << ".exit"
    result = run_script(script)

    expect(result).to match_array([
      "db > Executed.",
      "db > Executed.",
      "db > Executed.",
      "db > Tree:",
      "leaf (size 3)",
      " - 0 : 1",
      " - 1 : 2",
      " - 2 : 3",
      "db > "
    ])
  end

  it 'prints an error message if there is a duplicate id' do
    skip "Skipping this test"
    script = [
      "insert 1 user1 person1@example.com",
      "insert 1 user1 person1@example.com",
      "select",
      ".exit",
    ]
    result = run_script(script)
    expect(result).to match_array([
      "db > Executed.",
      "db > Error: Duplicate key.",
      "db > (1, user1, person1@example.com)",
      "Executed.",
      "db > ",
    ])
  end

  it 'insert max cells' do
      skip "Skipping this test"
      script = (1..13).map do |i|
        "insert #{i} user#{i} person#{i}@example.com"
      end
      script << ".exit"
      result = run_script(script)
  end
  it 'prints all rows in a multi-level tree' do
    skip "Skipping this test"
    script = []
    (1..15).each do |i|
      script << "insert #{i} user#{i} person#{i}@example.com"
    end
    script << "select"
    script << ".exit"
    result = run_script(script)

    expect(result[15...result.length]).to match_array([
      "db > (1, user1, person1@example.com)",
      "(2, user2, person2@example.com)",
      "(3, user3, person3@example.com)",
      "(4, user4, person4@example.com)",
      "(5, user5, person5@example.com)",
      "(6, user6, person6@example.com)",
      "(7, user7, person7@example.com)",
      "(8, user8, person8@example.com)",
      "(9, user9, person9@example.com)",
      "(10, user10, person10@example.com)",
      "(11, user11, person11@example.com)",
      "(12, user12, person12@example.com)",
      "(13, user13, person13@example.com)",
      "(14, user14, person14@example.com)",
      "(15, user15, person15@example.com)",
      "Executed.", "db > ",
    ])
  end

    it 'allows printing out the structure of a 4-leaf-node btree' do
      #skip "Skipping this test"

      script = [
        "insert 18 user18 person18@example.com",
        "insert 7 user7 person7@example.com",
        "insert 10 user10 person10@example.com",
        "insert 29 user29 person29@example.com",
        "insert 23 user23 person23@example.com",
        "insert 4 user4 person4@example.com",
        "insert 14 user14 person14@example.com",
        "insert 30 user30 person30@example.com",
        "insert 15 user15 person15@example.com",
        "insert 26 user26 person26@example.com",
        "insert 22 user22 person22@example.com",
        "insert 19 user19 person19@example.com",
        "insert 2 user2 person2@example.com",
        "insert 1 user1 person1@example.com",
        "insert 21 user21 person21@example.com",
        "insert 11 user11 person11@example.com",
        "insert 6 user6 person6@example.com",
        "insert 20 user20 person20@example.com",
        "insert 5 user5 person5@example.com",
        "insert 8 user8 person8@example.com",
        "insert 9 user9 person9@example.com",
        "insert 3 user3 person3@example.com",
        "insert 12 user12 person12@example.com",
        "insert 27 user27 person27@example.com",
        "insert 17 user17 person17@example.com",
        "insert 16 user16 person16@example.com",
        "insert 13 user13 person13@example.com",
        "insert 24 user24 person24@example.com",
        "insert 25 user25 person25@example.com",
        "insert 28 user28 person28@example.com",
            "insert 49 user49 person49@example.com",
            "insert 70 user70 person70@example.com",
            "insert 48 user48 person48@example.com",
            "insert 39 user39 person39@example.com",
            "insert 59 user59 person59@example.com",

        ".btree",
        ".exit",
      ]
      result = run_script(script)
        puts result

    end

  it 'allows printing out the structure of a 7-leaf-node btree' do
      script = []
      300.times do |i|
        script << "insert #{i} user#{i} person#{i}@example.com"
      end
    script << "select"
    script << ".btree"
    script << ".exit"
    result = run_script(script)
    puts result
  end
end
