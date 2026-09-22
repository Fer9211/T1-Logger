#include "LoggerI.h"
#include <iostream>
#include <orbsvcs/CosNamingC.h>

int main(int argc, char* argv[]) {
    try {
        // 1. Inicializa o ORB
        CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

        // 2. Obtem referencia para o RootPOA e ativa o POAManager
        CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
        PortableServer::POA_var root_poa = PortableServer::POA::_narrow(obj.in());
        
        PortableServer::POAManager_var poa_manager = root_poa->the_POAManager();
        poa_manager->activate();

        // 3. Cria a instancia do Servant (LoggerI)
        LoggerI* logger_servant = new LoggerI();
        PortableServer::ServantBase_var owner_def(logger_servant);

        // 4. Registra o servant no RootPOA e obtem sua referencia CORBA
        PortableServer::ObjectId_var oid = root_poa->activate_object(logger_servant);
        CORBA::Object_var ref = root_poa->id_to_reference(oid.in());
        T1Logger::Logger_var logger_ref = T1Logger::Logger::_narrow(ref.in());

        // 5. Publica a referencia no Servidor de Nomes (NameService)
        CORBA::Object_var naming_obj = orb->resolve_initial_references("NameService");
        CosNaming::NamingContext_var naming_context = CosNaming::NamingContext::_narrow(naming_obj.in());

        if (CORBA::is_nil(naming_context.in())) {
            std::cerr << "Erro: Servidor de Nomes (NameService) nao disponivel!" << std::endl;
            return 1;
        }

        // Define o nome "LoggerService" para o servico no NameService
        CosNaming::Name name;
        name.length(1);
        name[0].id = CORBA::string_dup("LoggerService");
        name[0].kind = CORBA::string_dup("");

        // Registra o servico
        naming_context->rebind(name, logger_ref.in());

        std::cout << "Servidor Logger iniciado com sucesso e registrado no NameService." << std::endl;
        std::cout << "Aguardando chamadas dos clientes..." << std::endl;

        // 6. Executa o loop principal do ORB para aguardar chamadas remotas
        orb->run();

        // 7. Limpeza final
        orb->destroy();

    } catch (const CORBA::Exception& e) {
        std::cerr << "Excecao CORBA no servidor: " << e << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Excecao desconhecida no servidor." << std::endl;
        return 1;
    }

    return 0;
}