#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <thread>
#include <chrono>

#if defined(__has_include)
  #if __has_include("LoggerC.h")
    #include "LoggerC.h"
  #elif __has_include("Logger.h")
    #include "Logger.h"
  #endif
#else
  #include "LoggerC.h"
#endif

#include <orbsvcs/CosNamingC.h>

// Função auxiliar para converter o enum Severidade para texto legível
const char* severidadeParaString(T1Logger::Severidade sev) {
    switch (sev) {
        case T1Logger::DEBUG:    return "DEBUG";
        case T1Logger::WARNING:  return "WARNING";
        case T1Logger::ERROR:    return "ERROR";
        case T1Logger::CRITICAL: return "CRITICAL";
        default:                 return "DESCONHECIDO";
    }
}

// Estrutura para representar um cliente simulado e seu evento
struct EventoSimulado {
    std::string nomeCliente;
    T1Logger::Severidade severidade;
    std::string endereco;
    CORBA::UShort pid;
    std::string mensagem;
};

int main(int argc, char* argv[]) {
    try {
        std::cout << "================================================================" << std::endl;
        std::cout << "      INICIANDO CLIENTE DE TESTES CORBA - LOGGER SERVICE        " << std::endl;
        std::cout << "================================================================" << std::endl;

        // 1. Inicializa o ORB
        CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

        // 2. Resolve referência para o Serviço de Nomes (Naming Service)
        std::cout << "[1] Conectando ao Servidor de Nomes (NameService)..." << std::endl;
        CORBA::Object_var naming_obj = orb->resolve_initial_references("NameService");
        CosNaming::NamingContext_var naming_context = CosNaming::NamingContext::_narrow(naming_obj.in());

        if (CORBA::is_nil(naming_context.in())) {
            std::cerr << "ERRO: Não foi possível obter o contexto do Naming Service!" << std::endl;
            return 1;
        }
        std::cout << "    -> Conectado com sucesso ao NameService." << std::endl;

        // 3. Localiza o LoggerService registrado pelo servidor
        std::cout << "[2] Localizando 'LoggerService' no Naming Service..." << std::endl;
        CosNaming::Name name;
        name.length(1);
        name[0].id = CORBA::string_dup("LoggerService");
        name[0].kind = CORBA::string_dup("");

        CORBA::Object_var obj = naming_context->resolve(name);
        T1Logger::Logger_var logger = T1Logger::Logger::_narrow(obj.in());

        if (CORBA::is_nil(logger.in())) {
            std::cerr << "ERRO: Referência do LoggerService é nula!" << std::endl;
            return 1;
        }
        std::cout << "    -> LoggerService localizado com sucesso!" << std::endl;

        // =====================================================================
        // TESTE 1: Testar o disparo da exceção EventNotFound em locate()
        // (Buscando severidades antes de enviar qualquer log para elas)
        // =====================================================================
        std::cout << "\n----------------------------------------------------------------" << std::endl;
        std::cout << "TESTE 1: Teste de Exceção EventNotFound em locate() vazio" << std::endl;
        std::cout << "----------------------------------------------------------------" << std::endl;
        std::cout << "Consultando locate() para severidades ainda não registradas..." << std::endl;

        try {
            std::cout << "Tentando locate(CRITICAL) no servidor limpo..." << std::endl;
            CORBA::String_var endr = logger->locate(T1Logger::CRITICAL);
            std::cerr << "  [FALHA] Não disparou exceção! Retornou endereço: " << endr.in() << std::endl;
        } catch (const T1Logger::EventNotFound& e) {
            std::cout << "  [SUCESSO] Exceção capturada conforme esperado!" << std::endl;
            std::cout << "  -> Mensagem da exceção: \"" << e.mensagem.in() << "\"" << std::endl;
        }

        try {
            std::cout << "Tentando locate(ERROR) no servidor limpo..." << std::endl;
            CORBA::String_var endr = logger->locate(T1Logger::ERROR);
            std::cerr << "  [FALHA] Não disparou exceção! Retornou endereço: " << endr.in() << std::endl;
        } catch (const T1Logger::EventNotFound& e) {
            std::cout << "  [SUCESSO] Exceção capturada conforme esperado!" << std::endl;
            std::cout << "  -> Mensagem da exceção: \"" << e.mensagem.in() << "\"" << std::endl;
        }

        // =====================================================================
        // TESTE 2: Testar o atributo verbose (leitura e escrita)
        // =====================================================================
        std::cout << "\n----------------------------------------------------------------" << std::endl;
        std::cout << "TESTE 2: Teste do Atributo 'verbose'" << std::endl;
        std::cout << "----------------------------------------------------------------" << std::endl;
        CORBA::Boolean verboseInicial = logger->verbose();
        std::cout << "Valor atual do atributo verbose: " << (verboseInicial ? "true (ativo)" : "false (inativo)") << std::endl;

        std::cout << "Garantindo que verbose esteja ativado (logger->verbose(true))..." << std::endl;
        logger->verbose(true);
        std::cout << "Novo valor de verbose confirmado: " << (logger->verbose() ? "true (ativo)" : "false (inativo)") << std::endl;

        // =====================================================================
        // TESTE 3: Simular múltiplos clientes enviando eventos fictícios via log(...)
        // =====================================================================
        std::cout << "\n----------------------------------------------------------------" << std::endl;
        std::cout << "TESTE 3: Simulação de Múltiplos Clientes Enviando Logs" << std::endl;
        std::cout << "----------------------------------------------------------------" << std::endl;

        std::vector<EventoSimulado> eventos = {
            {
                "Cliente 1 [Web Frontend]",
                T1Logger::DEBUG,
                "192.168.1.10:8080",
                1024,
                "Requisição HTTP GET /api/v1/status recebida com sucesso"
            },
            {
                "Cliente 2 [Auth Service]",
                T1Logger::WARNING,
                "192.168.1.25:3000",
                2048,
                "Múltiplas tentativas incorretas de login detectadas para o usuário 'admin'"
            },
            {
                "Cliente 3 [Database Worker]",
                T1Logger::ERROR,
                "192.168.1.40:5432",
                3072,
                "Falha ao sincronizar réplica secundária: timeout de conexão após 30s"
            },
            {
                "Cliente 4 [Firewall Gateway]",
                T1Logger::CRITICAL,
                "192.168.1.1:1500",
                4096,
                "Ataque SYN flood volumétrico detectado na interface externa WAN"
            },
            {
                "Cliente 5 [Proxy Reverso]",
                T1Logger::DEBUG,
                "192.168.1.1:1500",
                5120,
                "Novo túnel WebSocket estabelecido com sucesso na porta 1500"
            }
        };

        for (size_t i = 0; i < eventos.size(); ++i) {
            const auto& ev = eventos[i];
            CORBA::Long timestampAgora = static_cast<CORBA::Long>(time(nullptr));

            std::cout << "Enviando evento " << (i + 1) << "/" << eventos.size() << " de '" << ev.nomeCliente << "'..." << std::endl;
            std::cout << "  Severidade: " << severidadeParaString(ev.severidade)
                      << " | Endereço: " << ev.endereco
                      << " | PID: " << ev.pid
                      << " | Mensagem: " << ev.mensagem << std::endl;

            // Invocação assíncrona da operação oneway log(...)
            logger->log(
                ev.severidade,
                ev.endereco.c_str(),
                ev.pid,
                timestampAgora,
                ev.mensagem.c_str()
            );

            // Pequena pausa entre envios para visualização clara
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        std::cout << "\nTodos os eventos dos clientes simulados foram enviados com sucesso!" << std::endl;

        // =====================================================================
        // TESTE 4: Testar o método locate(s) para verificar o último endereço
        // =====================================================================
        std::cout << "\n----------------------------------------------------------------" << std::endl;
        std::cout << "TESTE 4: Verificação do Método locate() para cada Severidade" << std::endl;
        std::cout << "----------------------------------------------------------------" << std::endl;

        // Severidades e resultados esperados conforme os envios anteriores:
        // DEBUG foi enviado primeiro por 192.168.1.10:8080 e depois por 192.168.1.1:1500 -> Esperado: 192.168.1.1:1500
        // WARNING foi enviado por 192.168.1.25:3000 -> Esperado: 192.168.1.25:3000
        // ERROR foi enviado por 192.168.1.40:5432 -> Esperado: 192.168.1.40:5432
        // CRITICAL foi enviado por 192.168.1.1:1500 -> Esperado: 192.168.1.1:1500

        struct CasoLocate {
            T1Logger::Severidade severidade;
            std::string esperado;
        };

        std::vector<CasoLocate> casos = {
            { T1Logger::DEBUG,    "192.168.1.1:1500" },
            { T1Logger::WARNING,  "192.168.1.25:3000" },
            { T1Logger::ERROR,    "192.168.1.40:5432" },
            { T1Logger::CRITICAL, "192.168.1.1:1500" }
        };

        for (const auto& caso : casos) {
            try {
                // Em CORBA C++, o retorno char* deve ser armazenado em String_var para liberação automática
                CORBA::String_var enderecoObtido = logger->locate(caso.severidade);
                std::cout << "locate(" << severidadeParaString(caso.severidade) << "): "
                          << enderecoObtido.in();

                if (std::string(enderecoObtido.in()) == caso.esperado) {
                    std::cout << " -> [OK - Endereço mais recente correto!]" << std::endl;
                } else {
                    std::cout << " -> [DIVERGÊNCIA - Esperava '" << caso.esperado << "']" << std::endl;
                }
            } catch (const T1Logger::EventNotFound& e) {
                std::cerr << "  [ERRO INESPERADO] EventNotFound disparado para "
                          << severidadeParaString(caso.severidade) << ": " << e.mensagem.in() << std::endl;
            }
        }

        // =====================================================================
        // TESTE 5: Teste do modo silencioso (verbose = false)
        // =====================================================================
        std::cout << "\n----------------------------------------------------------------" << std::endl;
        std::cout << "TESTE 5: Teste de Desativação e Reativação de 'verbose'" << std::endl;
        std::cout << "----------------------------------------------------------------" << std::endl;

        std::cout << "Desligando verbose no servidor (logger->verbose(false))..." << std::endl;
        logger->verbose(false);

        std::cout << "Enviando log em modo silencioso (o servidor NÃO deve imprimir este evento na tela)..." << std::endl;
        logger->log(
            T1Logger::WARNING,
            "10.0.0.99:9999",
            9999,
            static_cast<CORBA::Long>(time(nullptr)),
            "Evento de teste silencioso (verbose desligado)"
        );

        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        std::cout << "Verificando se o locate atualizou mesmo em modo silencioso..." << std::endl;
        CORBA::String_var ultWarning = logger->locate(T1Logger::WARNING);
        std::cout << "locate(WARNING): " << ultWarning.in();
        if (std::string(ultWarning.in()) == "10.0.0.99:9999") {
            std::cout << " -> [OK - Estado interno atualizado corretamente!]" << std::endl;
        }

        std::cout << "Reativando verbose no servidor (logger->verbose(true))..." << std::endl;
        logger->verbose(true);

        std::cout << "Enviando log com verbose reativado (o servidor DEVE imprimir este evento)..." << std::endl;
        logger->log(
            T1Logger::DEBUG,
            "192.168.1.1:1500",
            8888,
            static_cast<CORBA::Long>(time(nullptr)),
            "Evento final com verbose reativado"
        );

        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        // =====================================================================
        // Conclusão
        // =====================================================================
        std::cout << "\n================================================================" << std::endl;
        std::cout << "  SUCESSO: TODOS OS TESTES DA INTERFACE FORAM CONCLUÍDOS!       " << std::endl;
        std::cout << "================================================================" << std::endl;

        // Finaliza o ORB
        orb->destroy();

    } catch (const CORBA::Exception& e) {
        std::cerr << "Exceção CORBA capturada no cliente: " << e << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Exceção padrão capturada no cliente: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Exceção desconhecida capturada no cliente." << std::endl;
        return 1;
    }

    return 0;
}
