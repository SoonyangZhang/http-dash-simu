#!/usr/bin/python
import time
import sys
import os
import threading
import subprocess
import signal

from mininet.cli import CLI
from mininet.log import lg, info
from mininet.node import Node
from mininet.util import quietRun
from mininet.net import Mininet
from mininet.link import TCLink
from mininet.link import Intf  
from mininet.node import OVSSwitch,Node

def test_adaptation_algorithm(algo):
    bottleneckbw=3
    nonbottlebw=500;
    rtt=40*3/2
    buffer_size =bottleneckbw*1000*rtt/(1500*8) 
    net = Mininet( cleanup=True )
    h1 = net.addHost('h1',ip='10.0.0.1')
    h2 = net.addHost('h2',ip='10.0.0.2')
    s1 = net.addSwitch( 's1' )
    c0 = net.addController('c0')
    net.addLink(h1,s1,intfName1='h1-eth0',intfName2='s1-eth0',cls=TCLink , bw=nonbottlebw, delay='10ms', max_queue_size=10*buffer_size)
    net.addLink(s1,h2,intfName1='s1-eth1',intfName2='h2-eth0',cls=TCLink , bw=bottleneckbw, delay='10ms', max_queue_size=buffer_size)    
    net.build()
    net.start()
    time.sleep(1)
    server_p=h2.popen("./t_dash_server -h 10.0.0.2 -p 3333")
    cmd="./t_dash_client -r 10.0.0.2  -p 3333 -a  %s"%algo
    client_p=h1.popen(cmd);
    possion_p=h2.popen("./possion_sender 10.0.0.1 1234")
    while True:
        ret=subprocess.Popen.poll(client_p)
        if ret is None:
            continue
        else:
            break
    os.killpg(os.getpgid(server_p.pid),signal.SIGTERM)
    os.killpg(os.getpgid(possion_p.pid),signal.SIGTERM)
    server_p.wait()  
    possion_p.wait()
    net.stop()
if __name__ == '__main__':
    algorithms=["festive","panda","tobasco","osmp","raahs","fdash","sftm","svaa"]
    for i in range(len(algorithms)):
        test_adaptation_algorithm(algorithms[i])
        print("done: "+algorithms[i])
