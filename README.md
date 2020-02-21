# Projeto de Compiladores
O que vai para o CVS: pasta src após `make clean`
Sugestão de commit message no CVS: hash/ID do último commit no git heh

The Makefile is magic and does everything

## Obter cópia do código do CVS (in case all goes to shit for some reason)
```sh
CVS_RSH=ssh cvs -d :ext:ist189409@sigma02.tecnico.ulisboa.pt:/afs/ist.utl.pt/groups/leic-co/co20/cvs/019 co -d src_cvs og
```

## Git -> CVS
`./update_cvs.sh`
Caso haja ficheiros não ignorados, por adicionar, ele dá o comando necessário e pára (sem o executar, para poder ser revisto).
Ele não faz commit sozinho, mas dá como output o comando para fazer commit no cvs.
