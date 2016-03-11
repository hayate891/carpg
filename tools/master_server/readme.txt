connect:
mp_masterserver - config var

struct Client
{
	Adr;
	bool is_admin, ok;
	float timer;
}
c -> ID_HELLO

Calling Connect and not calling SetMaximumIncomingConnections acts as a dedicated client.
GetLastPing
GetLowestPing
SetSplitMessageProgressInterval
hardcore server


SERVER
in config:
max_connections = 32 ?
secret = "pswd"

Po³¹czenie: dodaje client, ok = false, ma 5 sekund na wys³anie ID_HELLO potem jest od³¹czany


--------------------------------------------------------------------------------
KOMENDY
ID_HELLO - wysy³ane przez klienta z wersj¹ gry i protoko³u
w przysz³oœci bêdzie mo¿na dodaæ login i has³o
serwer odsy³a ID_HELLO
0 - ok
1 - jest nowa wersja (+dword wersja) - to ma wy¿szy priorytet ni¿ wersja protoko³u
2 - z³a wersja protoko³u
3 - broken
Serwer ustawia ok = true

ID_HOST - klient za³o¿y³ serwer (nazwa, max graczy, flagi)

ID_HOST_STOP

ID_LIST - klient chce pobieraæ listê serwerów

ID_LIST_STOP - klient chce przestaæ pobieraæ listê serwerów

ID_JOIN

ID_LOGIN loguje jako admin, ustawia is_admin = true
klient -> secret
server -> ID_LOGIN 0 lub 1 (0 - nieprawid³owy)

ID_CMD
	restart
	shutdown
	