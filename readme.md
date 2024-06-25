# Network Benchmark

## Run

* upstream
```
<udp|tcp>_upstream --server.host=${host1} --server.port=${port1} --cpu.freq=${profile}
```

* server
```
<udp|tcp>_server --server.host=${host2} --server.port=${port2} --conn.host=${host1} --conn.port=${port1} --cpu.freq=${profile}
```

* client
```
<udp|tcp>_client --conn.host=${host2} --conn.port=${port2} --bind.host=${host1} --cpu.freq=${profile}
```

## analysis

```
python network_latency_analysis.py -i <records-dir>
```
