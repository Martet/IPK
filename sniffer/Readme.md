# Hinfosvc

A packet sniffer written in c++ using the libpcap library.

## Compilation

Only standard unix libraries and libpcap are used and a Makefile is included. Compile running the `make` command, a `ipk-sniffer` binary file will be created.

## Usage

Start the `ipk-sniffer` with the desired arguments and captured packets will be printed out to stdout.

```
./ipk-sniffer [-i interface | --interface interface] {-p ­­port} {[--tcp|-t] [--udp|-u] [--arp] [--icmp] } {-n num}
```

Where:

* `interface` - the name of the desired interface to sniff on, if not enter, output available interfaces
* `port` - desired port to sniff on (if not entered, sniff on all ports)
* `num` - number of captured packets before program ends
* `tcp`, `udp`, `arp`, `icmp` - capture only this type of packet

## Examples:

```
./ipk-sniffer -i eth0 -p 23 --tcp -n 2
./ipk-sniffer -i eth0 --udp
./ipk-sniffer -i eth0 -n 10      
./ipk-sniffer -i eth0 -p 22 --tcp
```

## Example output:

```
$./ipk-sniffer -i eth0

timestamp: 2021-03-19T18:42:52+01:00
src MAC: 00:1c:2e:92:03:80
dst MAC: 00:1b:3f:56:8a:00
frame length: 512 bytes
src IP: 147.229.13.223
dst IP: 10.10.10.56
src port: 4093
dst port: 80

0x0000:  00 19 d1 f7 be e5 00 04  96 1d 34 20 08 00 45 00  ........ ..4 ..
0x0010:  05 a0 52 5b 40 00 36 06  5b db d9 43 16 8c 93 e5  ..R[@.6. [..C....
0x0020:  0d 6d 00 50 0d fb 3d cd  0a ed 41 d1 a4 ff 50 18  .m.P..=. ..A...P.
0x0030:  19 20 c7 cd 00 00 99 17  f1 60 7a bc 1f 97 2e b7  . ...... .`z.....
0x0040:  a1 18 f4 0b 5a ff 5f ac 07 71 a8 ac 54 67 3b 39  ....Z._. .q..Tg;9
0x0050:  4e 31 c5 5c 5f b5 37 ed  bd 66 ee ea b1 2b 0c 26  N1.\_.7. .f...+.&
0x0060:  98 9d b8 c8 00 80 0c 57  61 87 b0 cd 08 80 00 a1  .......W a.......
```

## Author

Martin Zmitko (xzmitko@stud.fit.vutbr.cz)

