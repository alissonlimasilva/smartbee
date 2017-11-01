#!/usr/bin/python

""" An example of a Linux daemon written in Python.



Based on http://www.jejik.com/articles/2007/02/a_simple_unix_linux_daemon_in_python/



The changes are:

1 - Uses file open context managers instead of calls to file().

2 - Forces stdin to /dev/null. stdout and stderr go to log files.

3 - Uses print instead of sys.stdout.write prior to pointing stdout to the log file.

4 - Omits try/excepts if they only wrap one error message w/ another.



i - http://stackoverflow.com/questions/3263672/python-the-difference-between-sys-stdout-write-and-print

"""

import atexit

import requests

import json

import datetime

import serial

import os

import signal

import sys

import time



class Daemon(object):

    """ Linux Daemon boilerplate. """

    def __init__(self, pid_file,

                 stdout='/var/log/smartbee/smartbee_out.log',

                 stderr='/var/log/smartbee/smartbee_err.log'):

        self.stdout = stdout

        self.stderr = stderr

        self.pid_file = pid_file



    def del_pid(self):

        """ Delete the pid file. """

        os.remove(self.pid_file)



    def daemonize(self):

        """ There shined a shiny daemon, In the middle, Of the road... """

        # fork 1 to spin off the child that will spawn the deamon.

        if os.fork():

            sys.exit()



        # This is the child.

        # 1. cd to root for a guarenteed working dir.

        # 2. clear the session id to clear the controlling TTY.

        # 3. set the umask so we have access to all files created by the daemon.

        os.chdir("/")

        os.setsid()

        os.umask(0)



        # fork 2 ensures we can't get a controlling ttd.

        if os.fork():

            sys.exit()



        # This is a child that can't ever have a controlling TTY.

        # Now we shut down stdin and point stdout/stderr at log files.



        # stdin

        with open('/dev/null', 'r') as dev_null:

            os.dup2(dev_null.fileno(), sys.stdin.fileno())



        # stderr - do this before stdout so that errors about setting stdout write to the log file.

        #

        # Exceptions raised after this point will be written to the log file.

        sys.stderr.flush()

        with open(self.stderr, 'a+', 0) as stderr:

            os.dup2(stderr.fileno(), sys.stderr.fileno())



        # stdout

        #

        # Print statements after this step will not work. Use sys.stdout

        # instead.

        sys.stdout.flush()

        with open(self.stdout, 'a+', 0) as stdout:

            os.dup2(stdout.fileno(), sys.stdout.fileno())



        # Write pid file

        # Before file creation, make sure we'll delete the pid file on exit!

        atexit.register(self.del_pid)

        pid = str(os.getpid())

        with open(self.pid_file, 'w+') as pid_file:

            pid_file.write('{0}'.format(pid))



    def get_pid_by_file(self):

        """ Return the pid read from the pid file. """

        try:

            with open(self.pid_file, 'r') as pid_file:

            	pid = int(pid_file.read().strip())

            return pid

        except IOError:

            return



    def start(self):

        """ Start the daemon. """

        print "Starting..."

        if self.get_pid_by_file():

            print 'PID file {0} existe. Ja existe um deamon executando?'.format(self.pid_file)

            sys.exit(1)



        self.daemonize()

        self.run()



    def stop(self):

        """ Stop the daemon. """

        print "Stopping..."

        pid = self.get_pid_by_file()

        if not pid:

            print "PID file {0} nao existe. O deamon nao esta rodando?".format(self.pid_file)

            return



        # Time to kill.

        try:

            while 1:

                os.kill(pid, signal.SIGTERM)

                time.sleep(0.1)

        except OSError as err:

            if 'No such process' in err.strerror and os.path.exists(self.pid_file):

                os.remove(self.pid_file)

            else:

                print err

                sys.exit(1)



    def restart(self):

        """ Restart the deamon. """

        self.stop()

        self.start()





    def run(self):

        """ The main loop of the daemon. """

	#Variaveis globais

	porta = "/dev/ttyACM0"

	velocidade = 57600

	urlFiware = "http://35.199.66.232:8080/webservice/coleta/add"
	#CHAVES DE CANAL
	COLMEIA1='ODQHRWU1WIPGH2CK'
	COLMEIA2='Z63X9L46JZFPDUWZ'
	COLMEIA3='NCFLS5NZV3MPWKAU'
	

	try:

		#Conexao com porta serial
		conexao = serial.Serial(porta, velocidade);

		#Limpa o buffer da porta serial para nao ler valores errados
		conexao.flushInput()
		

		while 1:

			leitura = conexao.readline()
#			leitura = "1 24 54 12 12"
			valores = leitura.split(' ')

			if leitura != "" and len(valores) == 5:

				headers = {
					'Content-Type':'application/json'
				}
				#Preparacao dos dados para enviar

				data1 = open('/etc/smartbee/json.json','r')

				data = data1.read()

				data1.close()

				
				#Alterar o atributos principais do JSON

				#IdColmeia

				data = data.replace('$1', valores[0])

				#Temperatura				

				data = data.replace('$2', valores[1])

				#Umidade

				data = data.replace('$3', valores[2])

				#Tensao Sensores

				data = data.replace('$4', valores[3])

				#Tensao Repetidores
				data = data.replace('$5', valores[4])

				#Consertando erros no arquivo JSON

				data = json.dumps(data)

				data = data.replace('\\n', '')

				data = data.replace('\\', '')

				data = data.replace('\"{', '{')
				data = data.replace('r\"}]}]','\"}]}]')

				data = data.replace('}\"', '}')

				status_requisicao = ''
				with open('/etc/smartbee/json.txt', 'a') as arq:
                                        arq.write(data+'\n')

						
				with open('/etc/smartbee/leituras.txt', 'a') as arq:

					arq.write(leitura+'\n')

				status_requisicao = ''

				

				#ENVIANDO REQUISICAO PARA THINKSPEAK

				id_key='';

				if (valores[0] == 1):

					id_key = COLMEIA1

				elif (valores[0] == 2):			

					id_key = COLMEIA2
				elif (valores[0] == 3):
					id_key = COLMEIA3
				try:
					r = requests.post(urlFiware, headers=headers, data=data)
					#ENVIA ATUALIZACAO DE VALORES THINKSPEAK
					url="http://api.thingspeak.com/update?api_key=%s&field1=%s&field2=%s&field3=%s&field4=%s" %(id_key,valores[1], valores[2], valores[3], valores[4], valores[5])
					r = requests.get(url)
				except requests.exceptions as err:
					with open('/var/log/smartbee/smartbee_err.log', 'a') as arq:
			                        arq.write(str(err)+'\n')
				except Exception as err:
                                        with open('/var/log/smartbee/smartbee_err.log', 'a') as arq:
						arq.write(str(err)+'\n')
			
	except Exception as err:

		with open('/var/log/smartbee/smartbee_err.log', 'a') as arq:

			arq.write(str(err)+'\n')

if __name__ == '__main__':

    if len(sys.argv) < 2:

        print "Use: {0} start|stop|restart".format(sys.argv[0])

        sys.exit(2)



    daemon = Daemon('/tmp/daemon_example.pid')

    if 'start' == sys.argv[1]:

        daemon.start()

    elif 'stop' == sys.argv[1]:

        daemon.stop()

    elif 'restart' == sys.argv[1]:

        daemon.restart()

    else:

        print "Comando desconhecido '{0}'".format(sys.argv[1])

        sys.exit(2)