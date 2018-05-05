all:
	make -C libcompute/
	make -C src/
	cp src/infractus .
	./pluginbuild.sh all
	
clean:
	rm src/*.o
	rm src/infractus
	rm libcompute/*.o
	./pluginbuild.sh cleanall
	rm infractus
