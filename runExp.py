import paramiko
from scp import SCPClient

hostname = '140.112.20.243'
port = 10073
# hostname = '192.168.48.73'
# port = 22

username = 'orange'
password = 'irislab123'



SPICE_FILE_NAME = ["POWER_1.sp", "POWER_2.sp", "POWER_3.sp"]
spice_result_file = [s.replace('.sp', '.lis') for s in SPICE_FILE_NAME]

local_file = ["outputs/" + s for s in SPICE_FILE_NAME]

remote_path = "PowerS/"
remote_file = [remote_path + s for s in SPICE_FILE_NAME]
result_file = [remote_path + s for s in spice_result_file]
local_result = ["exp/" + s for s in spice_result_file]

# Create SSH and SCP clients
ssh = paramiko.SSHClient()
ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
ssh.connect(hostname, port, username, password)


with SCPClient(ssh.get_transport()) as scp:
    for i in range(len(SPICE_FILE_NAME)):
        scp.put(local_file[i], remote_path)

# Create SSH client
client = paramiko.SSHClient()
client.set_missing_host_key_policy(paramiko.AutoAddPolicy())

try:
    client.connect(hostname, port, username, password)

    commands = [
        "source /nfs/home/cad/synopsys/CIC/hspice.cshrc",
    ]

    for i in range(len(SPICE_FILE_NAME)):
        commands.append("hspice -i " + remote_file[i] + " -o " + result_file[i])


    full_command = ' ; '.join(commands)  # or use ';' if you don't need to stop on error
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
        for i in range(len(SPICE_FILE_NAME)):
            scp.get(result_file[i], local_result[i])
        ssh.close()

finally:
    client.close()