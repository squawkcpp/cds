# Dockerfile to install content directory server (CDS)
FROM base/archlinux:latest

ARG CDS_TAG_VERSION=master

RUN pacman -Syu --noconfirm && \
    pacman --noconfirm -S boost poppler opencv doxygen hiredis libev hdf5 make   

COPY build/cds_$CDS_TAG_VERSION.tar.gz /cds.tar.gz
#RUN /usr/sbin/tar xfz /cds.tar.gz 

ADD docker/cds.sh /cds.sh
RUN chmod +x /*.sh

EXPOSE 8000

CMD ["/bin/bash"]
