#!/bin/sh

[ -z "$log_level" ] && log_level=DEBUG
cat <<EOF
log4j.rootLogger=$log_level,R
log4j.appender.R=org.apache.log4j.ConsoleAppender
log4j.appender.R.layout=org.apache.log4j.PatternLayout
log4j.appender.R.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss.SSS} - %-5p [%X{pid}] [%x] %c - [%F:%L] %m%n
EOF
