# ARP project 
The goal is to simulate a network of multi-process systems, all identical, each one running on a machine in the same LAN, connected through sockets, exchanging a data token at a specified and measured speed. 

Everything is implemented on a single machine all processes, simulating a distributed implementation. An artificial delay (it will be called waiting time) is used to mimic the network communication delays. Each machine contains 4 Posix processes:
- P: Receives tokens, computes and sends an updated token after a given delay. 
- S: Receives conseole messages as Posix signals. 
- G: Receives tokens, sends them to P.
- L: Writes log file.

## Files

### ARP.C and ARP.h
Each machine contains 4 Posix processes:
- P: Receives tokens, computes and sends an updated token after a given delay. Computation: new token = received token + DT x (1. - (received token)^2/2) x 2 pi x RF. Connected to S through pipe (RW). Connected to G through pipe (RW)and through socket (W). Connected to L through pipe (W).
- S: Receives conseole messages as Posix signals. Signals are for carrying out the following actions:
  - start (receiving tokens, computing, sending tokens)
  - stop ((receiving tokens, computing, sending tokens)
  - dump log: outputs the contents of the log file, separating the wave from the actions.
- L: Writes log file.

Note: The token is a floating point number between [-1,+1] + time stamp with absolute time of when P writes on the socket. 

### G.c
G: Receives tokens, sends them to P.


### Config file
The configuration file contains in text format the necessary parameters: IP address, reference frequency RF of the token wave (1), waiting rime (1 mu_s). (Then these values must change in order to find the maximum RF frequency and the minimum waiting time compatible with the implementation.)

### Log file
The log file holds a series of text lines in couples:
<timestamp> <from G | from S > <value>
<timestamp> <sent value> (a sample of the wave)
  

n.b. A graphical interface for inspecting the generated wave is strongly recommended (e.g. using matlab or excel or
another app on the log file). Please give the processor and opsys types.
