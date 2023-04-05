import getpass
import socket


class LogUtils:

    @staticmethod
    def user_name() -> str:
        return getpass.getuser()

    @staticmethod
    def host_name() -> str:
        return socket.gethostname()
