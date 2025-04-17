if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <sources> <caches>"
    exit 1
fi
docker build -t xpansion/centos7 .
docker run \
--mount type=bind,src=$1,dst=/mnt/sources \
--mount type=bind,src=$2,dst=/mnt/caches \
xpansion/centos7 \
/mnt/sources /mnt/caches