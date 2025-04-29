import machine
import utime
import dht

sensor = dht.DHT22(machine.Pin(4))
vermelho = machine.Pin(13, machine.Pin.OUT) 
yellow = machine.Pin(15, machine.Pin.OUT) 
verde = machine.Pin(14, machine.Pin.OUT) 

while True:
    sensor.measure() 
    h = sensor.humidity()
    if (h>40):
        vermelho.value(0)
        verde.value(1)
        yellow.value(0)
    elif (h==0):
        vermelho.value(0)
        verde.value(0)
        yellow.value(1)
    else:
        vermelho.value(1)
        verde.value(0)
        yellow.value(0)
        
    print("Umidade: ")
    print(h)
    utime.sleep(2)

