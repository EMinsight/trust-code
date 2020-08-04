# Echo client program
import socket
import sys
try:
    sys.dont_write_bytecode = True
except:
    pass

HOST = ''    # The remote host
from port import PORT
from job_manager import MAX_SEND_RCV_SIZE
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
try:
    s.connect((HOST, PORT))
except ConnectionRefusedError:
    print("client.py cannot connect to server - exiting.")
    sys.exit(1)
s.sendall(bytes(' '.join(sys.argv[1:]), "UTF-8"))
while 1:
    try:
        data = s.recv(MAX_SEND_RCV_SIZE).decode("UTF-8")
        if data:
            break
    except socket.error:
        print("client.py: socket was closed - exiting ...")
        sys.exit(1)
s.close()
print(data)
if (data.startswith("EXIT")):  # Something went wrong on server side: stop, kill or shutdown
   sys.exit(1)
