PROJECT := obd2_rpi

.PHONY: all clean run

all:
	cmake -S $(PROJECT) -B $(PROJECT)/build
	make -C $(PROJECT)/build -j$$(nproc)
	@echo "[OK] Binario: $(PROJECT)/bin/obd2_rpi"

run:
	sudo $(PROJECT)/bin/obd2_rpi

clean:
	rm -rf $(PROJECT)/build $(PROJECT)/bin
