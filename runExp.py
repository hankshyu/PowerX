import paramiko
from scp import SCPClient

hostname = '140.112.20.243'
port = 10073
username = 'orange'
password = 'irislab123'


SPICE_FILE_NAME = "POWER1.sp"
spice_result_file = SPICE_FILE_NAME.replace('.sp', '.lis')

local_file = "outputs/" + SPICE_FILE_NAME

remote_path = "PowerS/"
remote_file = remote_path + SPICE_FILE_NAME
result_file = remote_path + spice_result_file
local_result = "exp/" + spice_result_file

# Create SSH and SCP clients
ssh = paramiko.SSHClient()
ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
ssh.connect(hostname, port, username, password)

with SCPClient(ssh.get_transport()) as scp:
    scp.put(local_file, remote_path)

# Create SSH client
client = paramiko.SSHClient()
client.set_missing_host_key_policy(paramiko.AutoAddPolicy())

try:
    client.connect(hostname, port, username, password)

    commands = [
        "source /nfs/home/cad/synopsys/CIC/hspice.cshrc",
        "hspice " + remote_file + " > " + result_file
        
    ]
    full_command = ' && '.join(commands)  # or use ';' if you don't need to stop on error
    stdin, stdout, stderr = client.exec_command(f'tcsh -c "{full_command}"')

    print("STDOUT:")
    print(stdout.read().decode())

    print("STDERR:")
    print(stderr.read().decode())


    # Create SSH and SCP clients
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    ssh.connect(hostname, port, username, password)

    with SCPClient(ssh.get_transport()) as scp:
        scp.get(result_file, local_result)
    ssh.close()

finally:
    client.close()