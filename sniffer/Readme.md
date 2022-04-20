# Hinfosvc

A packet sniffer written in c++ using the libpcap library.

## Compilation

Only standard unix libraries are used and a Makefile is included, so the only prerequisite is gcc. Compile running the `make` command, a `hinfosvc` binary file will be created.

## Usage

Start the `hinfosvc` binary with the desired port as an argument and leave it running on the background. 

```
./hinfosvc <PORT>
```

There are 3 endpoints, to which GET requests can be sent and the corresponding information will be returned as a plaintext response. Those endpoints are:

* `/hostname` - gets the hostname of the server
* `/cpu-name` - gets the name of the cpu of the server
* `/load` - gets the current cpu load of the server

## Examples:

```
$ curl localhost:12345/hostname
merlin.fit.vutbr.cz

$ curl localhost:12345/cpu-name
Intel(R) Xeon(R) Silver 4214R CPU @ 2.40GHz

$ curl localhost:12345/load
12%
```

## Author and additional information

Martin Zmitko (xzmitko@stud.fit.vutbr.cz)

Cpu load is calculated with a calculation taken from https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux

