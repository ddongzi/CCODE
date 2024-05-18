https://codepr.github.io/posts/sol-mqtt-broker/

十分钟快速入门MQTT协议
https://www.emqx.com/zh/blog/get-started-with-mqtt-in-ten-mins

remaining length 
这种编码方式的特点是用一个或多个字节来表示一个整数值，其中每个字节的最高位用来表示是否还有后续字节，其余 7 位用来存储整数值的一部分。
在这个编码方式中，长度值被分割成多个字节，每个字节的最高位用来表示是否还有后续字节，如果有，则置为 1，否则置为 0。剩余的 7 位用来存储长度值的一部分。

为什么使用 128？因为 128 的二进制表示为 10000000，其中最高位为 1，其余位为 0。因此，用来表示是否还有后续字节非常适合。如果待编码的长度值小于 128，只需要一个字节就可以表示完整的值；如果大于等于 128，就需要多个字节来表示。

| field   | size (bits) | offset | description |
|---------|-------------|--------|-------------|
| type    | 1(4bits)           | 0      |             |
| length  | 1           | 1      |             |
| Protocol name length  | 2           | 2      |             |
| Protocol name (MQTT)  | 4           | 4      |             |
| Protocol level  | 1           | 8      |             |
| Connect flags  | 1           | 9      |             |
| Keepalive  | 2           | 10      |     maximum value is 18 hr 12 min 15 seconds        |
| Client ID length  | 2           | 12      |             |
| Client ID	  | 6           | 14      |             |
| Username length  | 2           | 20      |             |
| Username   | 5           | 22      |             |
| Password length  | 2           | 27      |             |
| Password length  | 5           | 29      |             |