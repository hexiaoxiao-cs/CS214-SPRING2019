all: WTF WTFserver WTFtest


clean:
	rm -rf build

WTF:
	mkdir -p build
	cd build ;\
	cmake .. ;\
	make WTF

WTFserver:
	mkdir -p build
	cd build ;\
	cmake .. ;\
	make WTFserver

WTFtest:
	mkdir -p build
	cd build ;\
	cmake .. ;\
	make WTFtest
	cp tt_test_payload build/tt_test_payload
	cp tm_test_payload build/tm_test_payload
	echo "Please goto build folder to execute"
