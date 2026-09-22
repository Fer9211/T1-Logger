#include "LoggerC.h"
#include <orbsvcs/CosNamingC.h>
#include <iostream>
#include <ctime>

int main(int argc, char* argv[]) {
    try {
        // 1. Inicializa o ORB
        CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

        // 2. Conecta ao Servidor de Nomes (NameService)
        std::cout << "[Cliente] Conectando ao NameService..." << std::endl;
        CORBA::Object_var naming_obj = orb->resolve_initial_references("NameService");
        CosNaming::NamingContext_var naming_context = CosNaming::NamingContext::_narrow(naming_obj.in());

        if (CORBA::is_nil(naming_context.in())) {
            std::cerr << "Erro: NameService indisponivel!" << std::endl;
            return 1;
        }

        // 3. Localiza o servico "LoggerService" registrado pelo servidor
        CosNaming::Name name;
        name.length(1);
        name[0].id = CORBA::string_dup("LoggerService");
        name[0].kind = CORBA::string_dup("");

        CORBA::Object_var obj = naming_context->resolve(name);
        T1Logger::Logger_var logger = T1Logger::Logger::_narrow(obj.in());

        if (CORBA::is_nil(logger.in())) {
            std::cerr << "Erro: Referencia do LoggerService e nula!" << std::endl;
            return 1;
        }
        std::cout << "[Cliente] Conectado ao LoggerService com sucesso!\n" << std::endl;

        // =====================================================================
        // TESTE 1: Testar exceção EventNotFound em locate() sem eventos prévios
        // =====================================================================
        std::cout << "--- TESTE 1: Testando locate() vazio (deve lancar excecao) ---" << std::endl;
        try {
            std::cout << "Tentando chamar locate(CRITICAL)..." << std::endl;
            CORBA::String_var endr = logger->locate(T1Logger::CRITICAL);
            std::cout << "FALHA: Nao disparou excecao. Retornou: " << endr.in() << std::endl;
        } catch (const T1Logger::EventNotFound& e) {
            std::cout << "SUCESSO: Excecao capturada conforme esperado!" << std::endl;
            std::cout << "-> Mensagem da excecao: \"" << e.mensagem.in() << "\"" << std::endl;
        }

        // =====================================================================
        // TESTE 2: Envio de eventos assíncronos (log) com dados fictícios
        // =====================================================================
        std::cout << "\n--- TESTE 2: Enviando eventos de log ficticios (assincronos) ---" << std::endl;

        std::cout << "Enviando log DEBUG..." << std::endl;
        logger->log(T1Logger::DEBUG, "192.168.1.10:8080", 1001, static_cast<CORBA::Long>(time(nullptr)), "Requisicao HTTP processada com sucesso");

        std::cout << "Enviando log WARNING..." << std::endl;
        logger->log(T1Logger::WARNING, "192.168.1.20:3000", 1002, static_cast<CORBA::Long>(time(nullptr)), "Consumo elevado de memoria");

        std::cout << "Enviando log ERROR..." << std::endl;
        logger->log(T1Logger::ERROR, "192.168.1.30:5432", 1003, static_cast<CORBA::Long>(time(nullptr)), "Falha de conexao com o banco de dados");

        std::cout << "Enviando log CRITICAL..." << std::endl;
        logger->log(T1Logger::CRITICAL, "192.168.1.40:1500", 1004, static_cast<CORBA::Long>(time(nullptr)), "Queda no link principal de rede");

        std::cout << "Todos os eventos foram enviados para o servidor!" << std::endl;

        // =====================================================================
        // TESTE 3: Consultar o último endereço de cada severidade (locate)
        // =====================================================================
        std::cout << "\n--- TESTE 3: Consultando locate() para cada severidade ---" << std::endl;
        
        CORBA::String_var endrDebug = logger->locate(T1Logger::DEBUG);
        std::cout << "locate(DEBUG)    -> " << endrDebug.in() << " (esperado: 192.168.1.10:8080)" << std::endl;

        CORBA::String_var endrWarning = logger->locate(T1Logger::WARNING);
        std::cout << "locate(WARNING)  -> " << endrWarning.in() << " (esperado: 192.168.1.20:3000)" << std::endl;

        CORBA::String_var endrError = logger->locate(T1Logger::ERROR);
        std::cout << "locate(ERROR)    -> " << endrError.in() << " (esperado: 192.168.1.30:5432)" << std::endl;

        CORBA::String_var endrCritical = logger->locate(T1Logger::CRITICAL);
        std::cout << "locate(CRITICAL) -> " << endrCritical.in() << " (esperado: 192.168.1.40:1500)" << std::endl;

        // =====================================================================
        // TESTE 4: Testar atualização do locate() ao receber novo evento
        // =====================================================================
        std::cout << "\n--- TESTE 4: Testando atualizacao do ultimo endereco no locate() ---" << std::endl;
        std::cout << "Enviando novo evento WARNING vindo de '10.0.0.99:9999'..." << std::endl;
        logger->log(T1Logger::WARNING, "10.0.0.99:9999", 2005, static_cast<CORBA::Long>(time(nullptr)), "Novo aviso de seguranca");

        endrWarning = logger->locate(T1Logger::WARNING);
        std::cout << "locate(WARNING) atualizado -> " << endrWarning.in() << " (esperado: 10.0.0.99:9999)" << std::endl;

        std::cout << "\n================================================================" << std::endl;
        std::cout << "       TODOS OS TESTES FORAM CONCLUIDOS COM SUCESSO!            " << std::endl;
        std::cout << "================================================================" << std::endl;

        // Finaliza o ORB
        orb->destroy();

    } catch (const CORBA::Exception& e) {
        std::cerr << "Excecao CORBA: " << e << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Excecao desconhecida no cliente!" << std::endl;
        return 1;
    }

    return 0;
}
