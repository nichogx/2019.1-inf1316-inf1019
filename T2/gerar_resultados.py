import time
import subprocess
import re

timeInicial = time.time()

tamanhoPaginas = [8, 16, 32]
tamanhoMemoria = [1, 2, 4, 8, 16]
arquivos = ["compressor", "matriz", "compilador", "simulador"]
tipos = ["nru", "lru", "novo"]

print("Gerando %d tabelas de resultado" % (len(tamanhoPaginas) * len(tamanhoMemoria)))
print("Isso deve demorar alguns minutos/horas")
print()

for pag in tamanhoPaginas:
	for mem in tamanhoMemoria:
		print("MEMÓRIA DE %dMB, PÁGINAS DE %dKB" % (mem, pag))

		print(20 * " " + " |             NRU               |             LRU               |             NOVO              | ")
		print(20 * " " + " | " + "  PF   | REWRITES | TEMPO (s) | " * 3)
		print(118 * ".")

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
