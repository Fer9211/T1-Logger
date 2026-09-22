#ifndef LOGGERI_H
#define LOGGERI_H

#include "LoggerS.h"
#include <map>
#include <string>
#include <mutex>

class LoggerI : public virtual POA_T1Logger::Logger {
private:
    // Guarda o último endereço recebido para cada severidade
    std::map<T1Logger::Severidade, std::string> ultimosEnderecos;
    std::mutex mtx;

public:
    LoggerI();
    virtual ~LoggerI();

    // Método assíncrono para registrar eventos de log
    virtual void log(
        T1Logger::Severidade severidade,
        const char* endereco,
        CORBA::UShort pid,
        CORBA::Long hora,
        const char* msg
    );

    // Método locate para buscar o último endereço da severidade
    virtual char* locate(T1Logger::Severidade severidade);
};

#endif // LOGGERI_H