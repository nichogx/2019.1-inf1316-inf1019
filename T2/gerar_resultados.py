import time
import subprocess
import re

timeInicial = time.time()

tamanhoPaginas = [32]
tamanhoMemoria = [8]
arquivos = ["compilador", "compressor", "matriz", "simulador"]
tipos = ["lru", "nru", "novo"]

for pag in tamanhoPaginas:
	for mem in tamanhoMemoria:
		print("MEMÓRIA DE %dMB, PÁGINAS DE %dKB" % (mem, pag))

		print(20 * " " + " |             LRU               |             NRU               |             NOVO              | ")
		print(20 * " " + " | " + "  PF   | REWRITES | TEMPO (s) | " * 3)
		print(118 * "-")

		for arq in arquivos:
			arq += ".log"

			strprt = "%-20s | " % arq

			for tipo in tipos:

				res = subprocess.check_output(["./simvirtual", tipo, arq, str(pag), str(mem)]).decode()
				pfaults = re.search("Page faults: (\\d+)", res).group(1)
				rewritten = re.search("Reescritas para a memória: (\\d+)", res).group(1)
				timetaken = re.search("TEMPO: (\\d+)", res).group(1)

				strprt += "%-6s | %-8s | %-9s | " % (pfaults, rewritten, timetaken)
				print(strprt, end="\r")

			print()
		print()


# printa tempo final
segundos = int(time.time() - timeInicial)
minutos = segundos // 60
segundos = segundos % 60
horas = minutos // 60
minutos = minutos % 60
print("Tempo total de execução: %dh %dmin %dseg" % (horas, minutos, segundos))
