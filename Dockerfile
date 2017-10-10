# Dockerfile to install content directory server (CDS)
FROM ubuntu:xenial

RUN apt-get -y update \
&& apt-get install -y wget

ADD build/cds_0.1.12.deb /cds_0.1.12.deb

RUN apt-get install -y libpcrecpp0v5 libimlib2 libavcodec-ffmpeg56 libavformat-ffmpeg56 libavutil-ffmpeg54 libpoppler-cpp0 libboost-filesystem1.58.0 \
        libcurl3 libhiredis0.13 libev4 libmagic1 libopencv-highgui2.4v5 \
&& dpkg -i /cds_0.1.12.deb

#RUN wget "https://github.com/squawkcpp/cds/releases/download/0.1.12/cds_0.1.12.deb" \
#&& apt-get install -y libpcrecpp0v5 libimlib2 libavcodec-ffmpeg56 libavformat-ffmpeg56 libavutil-ffmpeg54 libpoppler-cpp0 libboost-filesystem1.58.0 \
#        libcurl3 libhiredis0.13 libev4 libmagic1 libopencv-highgui2.4v5 \
#&& dpkg -i cds_0.1.12.deb

ADD script/cds.sh /cds.sh
RUN chmod +x /*.sh

EXPOSE 8000

CMD ["/cds.sh"]
