# Dockerfile to install content directory server (CDS)
FROM busybox:musl

ARG CDS_TAG_VERSION=master

ADD .build/squawk-cds_$CDS_TAG_VERSION.tar.gz /

ADD docker/cds.sh /cds.sh
RUN chmod +x /*.sh

EXPOSE 9001

CMD ["/cds.sh"]

