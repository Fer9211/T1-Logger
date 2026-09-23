#ifndef LOGGERI_H
#define LOGGERI_H

#include "LoggerS.h"
#include <map>
#include <string>
#include <mutex>

class LoggerI : public virtual POA_T1Logger::Logger {
private:
    // guarda ultimo endereço recebido p cada severidade
    std::map<T1Logger::Severidade, std::string> ultimosEnderecos;
    std::mutex mtx;

public:
    LoggerI();
    virtual ~LoggerI();

    // async p registrar eventos de log
    virtual void log(
        T1Logger::Severidade severidade,
        const char* endereco,
        CORBA::UShort pid,
        CORBA::Long hora,
        const char* msg
    );

    // p buscar ultimo endereço da severidade
    virtual char* locate(T1Logger::Severidade severidade);
};

#endif // LOGGERI_H