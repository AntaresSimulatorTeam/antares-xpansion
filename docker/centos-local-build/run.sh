docker build -t xpansion/centos7 .
docker run \
--mount type=bind,src=/home/marechaljas/CLionProjects/antares-xpansion,dst=/mnt/sources \
--mount type=bind,src=/home/marechaljas/CLionProjects/docker_caches,dst=/mnt/caches \
xpansion/centos7 \
/mnt/sources /mnt/caches