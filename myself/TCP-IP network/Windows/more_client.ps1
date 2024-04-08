$ip=(ipconfig.exe | findstr /i "ipv4").split(': ')[-1]
for ($i=0; $i -lt 50; $i++) {
    .\client.exe $ip 8888
}