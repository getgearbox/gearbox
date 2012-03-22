// -*- c++ -*-
#ifndef GEARBOX_LOGGER_H
#define GEARBOX_LOGGER_H

#include <log4cxx/logger.h>
#include <gearbox/core/Json.h>

namespace Gearbox {
    void log_init(const std::string & configFile);

#ifdef LOGCAT
#define GEARBOX_LOGGER log4cxx::Logger::getLogger(LOGCAT)
#else
#define GEARBOX_LOGGER log4cxx::Logger::getRootLogger()
#endif

  namespace logger {
    __attribute__((unused)) static char format_logger_buffer[512];
  }
}

#define _TRACE(msg) LOG4CXX_TRACE( GEARBOX_LOGGER, msg )
#define _DEBUG(msg) LOG4CXX_DEBUG( GEARBOX_LOGGER, msg )
#define _INFO(msg)  LOG4CXX_INFO( GEARBOX_LOGGER, msg )
#define _WARN(msg)  LOG4CXX_WARN( GEARBOX_LOGGER, msg )
#define _ERROR(msg) LOG4CXX_ERROR( GEARBOX_LOGGER, msg )
#define _FATAL(msg) LOG4CXX_FATAL( GEARBOX_LOGGER, msg )

#define _TRACEF(format, ...) { \
    snprintf( Gearbox::logger::format_logger_buffer, 512, format, ##__VA_ARGS__ ); \
    _TRACE(Gearbox::logger::format_logger_buffer); }
#define _DEBUGF(format, ...) { \
    snprintf( Gearbox::logger::format_logger_buffer, 512, format, ##__VA_ARGS__ ); \
    _DEBUG(Gearbox::logger::format_logger_buffer); }
#define _INFOF(format, ...)  { \
    snprintf( Gearbox::logger::format_logger_buffer, 512, format, ##__VA_ARGS__ ); \
    _INFO(Gearbox::logger::format_logger_buffer); }
#define _WARNF(format, ...)  { \
    snprintf( Gearbox::logger::format_logger_buffer, 512, format, ##__VA_ARGS__ ); \
    _WARN(Gearbox::logger::format_logger_buffer); }
#define _ERRORF(format, ...) { \
    snprintf( Gearbox::logger::format_logger_buffer, 512, format, ##__VA_ARGS__ ); \
    _ERROR(Gearbox::logger::format_logger_buffer); }
#define _FATALF(format, ...) { \
    snprintf( Gearbox::logger::format_logger_buffer, 512, format, ##__VA_ARGS__ ); \
    _FATAL(Gearbox::logger::format_logger_buffer); }


#endif // GEARBOX_LOGGER_H
