all: servidor cliente

servidor:
	@echo ">>> Compilando Servidor..."
	@$(MAKE) -C servidor

cliente:
	@echo ">>> Compilando Cliente..."
	@$(MAKE) -C cliente

clean:
	@echo ">>> Limpando Servidor..."
	@$(MAKE) -C servidor clean
	@echo ">>> Limpando Cliente..."
	@$(MAKE) -C cliente clean
	@rm -rf bin

.PHONY: all servidor cliente clean
