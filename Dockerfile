# Dockerfile to install content directory server (CDS)
FROM ubuntu:xenial

ARG CDS_TAG_VERSION=master

RUN apt-get -y update

ADD build/cds_$CDS_TAG_VERSION.deb /cds_$CDS_TAG_VERSION.deb

RUN apt-get install -y libpcrecpp0v5 libimlib2 libavcodec-ffmpeg56 libavformat-ffmpeg56 libavutil-ffmpeg54 libpoppler-cpp0 libboost-filesystem1.58.0 \
        libcurl3 libhiredis0.13 libev4 libmagic1 libopencv-highgui2.4v5 \
&& dpkg -i /cds_$CDS_TAG_VERSION.deb

ADD script/cds.sh /cds.sh
RUN chmod +x /*.sh

EXPOSE 8000

CMD ["/cds.sh"]
