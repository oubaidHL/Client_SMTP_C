# Directives pour le rendu du client SMTP

[[_TOC_]]

## Introduction

Le but de ce travail est d'approfondir et mettre en pratique

   * vos connaissances du cours C

   * vos connaissances du cours réseau, en particulier la notion de protocole (automate), la programmation TCP socket et le protocole SMTP

   * vos connaissances des cours OS et Linux (virtualisation, ligne de commande, commandes de base)

## Travail à rendre (rapport et logiciel)

Il y a un *code source* et un *rapport* à rendre, par groupe de 2.

Les *objectifs* du code source à coder et rendre sont:

   * coder un client SMTP en langage C sous plateforme POSIX (GNU/Linux) qui puisse, à partir
      d'informations de la *ligne de commande* envoyer un mail via
      le protocole SMTP, y compris la gestion des *erreurs* SMTP (fatales ou temporaires, avec notamment la gestion de la
      *réémission* en cas de grey-listing)

   * les arguments de la ligne de commande sont, dans l'ordre
      * e-mail expéditeur,
      * sujet du message (entre guillemets si multi-mots, cf bash)
      * nom de fichier du corps du message
      * nom de domaine DNS ou adresse IP du serveur de mail à utiliser
      * e-mail destinataire
      * argument optionnel: numéro de port (par défaut 25)

   * le client doit afficher un retour à l'utilisateur si l'e-mail a bien été
     envoyé ou afficher le message d'erreur SMTP dans le cas contraire.

   * en pratique, le client doit avoir été testé
      * avec un serveur local simulé par vous avec la commande netcat (nc), y compris erreurs fatales et temporaires (grey-listing)
      * sur le serveur de l'école ``smtprel.he-arc.ch`` avec expéditeur quelconque et votre adresse e-mail de destination
      * sur le serveur ``smtp.alphanet.ch`` avec destinataire votre adresse e-mail HE-Arc (but: observer
        l'anti-relaying et informer l'utilisateur de l'erreur fatale)
         * indication: utiliser le port 587 plutôt que 25 depuis l'Ecole ou Swisscom (firewall/proxy "intelligent")
         * destinataires: schaefer@alphanet.ch ou info@alphanet.ch (ou test@alphanet.ch pour recevoir au plus 1 réponse par jour si votre adresse From: est valable et que le sujet du message est test)
        (but: observer le grey-listing, attendre un peu et réessayer)
         * en cas d'erreur 5xx systématique, activez le VPN (*interne*; votre adresse dynamique xDSL est peut-être blacklistée)
      * les tests sont *documentés* dans le rapport (minimum: copie d'écran)

#PhasesDeveloppement
   * proposition de phases de développement
      * 1: réflexion sur une architecture d'automate du client SMTP
         * chacune des phases du protocole (connexion, envoi du HELO, envoi du MAIL FROM, envoi du RCPT TO, envoi du DATA, envoi des données, fin) peut être considérée comme un état d'un automate: dessinez ces états
         * les transitions entre les phases sont basées sur des *conditions*: des événements provenant du serveur, voire de la couche 4 (p.ex. après connexion, le
           220 du serveur permet d'envoyer le HELO, etc): indiquez les conditions des transitions et ce que le client envoie en réponse (p.ex. sous la forme condition/réaction ou plus généralement: attendu/envoyé)
         * expliquez ce que vous faites si le code attendu n'est pas obtenu et où vous le faites pour éviter la duplication de code (D.R.Y.)
            * cas spécial: code 4xx
            * autres cas (erreurs)
         * proposez une implémentation C de l'automate client SMTP, basée sur ``switch variable_etat`` et une boucle "infinie": voir l'[exemple d'automate de protocole réalisé en C](automate.c)
      * 2: envoi d'un mail dont le contenu est "hard-codé", en parsant la ligne de commande pour les
         autres éléments, en tenant compte du protocole SMTP avec le serveur, mais en ignorant les erreurs; en
         affichant le déroulé à l'écran
      * 3: lecture d'un fichier spécifié pour le contenu
      * 4: parsing des codes d'erreurs du serveur et plantage avec informations à l'écran si pas attendu
      * 5: implémentation de la fonction de ré-essai pour le grey-listing
         * en cas de code 4xx ou en cas de réussite, la session SMTP est terminée proprement, la connexion est fermée (avant le réessai éventuel)

Un *rapport* en *PDF* doit accompagner le logiciel, avec:
   * une documentation utilisateur très simplifiée (comment utiliser votre programme)
   * le schéma d'automate simplifié (peut être fait à la main et scanné)
   * une explication dans les grandes lignes de la structure du logiciel (fonctions, structures de données), y compris la *notion d'automate*
   * une documentation simple de génération du logiciel (comment vous compilez le logiciel, peut être une seule ligne!)
   * une documentation des observations effectuées (notamment le greylisting) et des résultats obtenus (copies d'écran du diagnostic de votre programme en cas d'envoi de mail avec réessai)

Le rapport ne doit pas être structuré selon les recommandations usuelles (introduction, conclusion,
table des matières et références ne sont _pas_ nécessaires).

## Moyens

Vous pouvez repartir du [corrigé de l'exercice client HTTP](04_Couche_transport/corriges/client-http-corrige.tar.gz).

## Evaluation, déroulement et directives

   * délai: le dimanche de la semaine de la fin des cours, 23:59 au plus tard
      * envoyez par e-mail une archive zip dont le nom est ``Reseaux-Nom1-Nom2.zip`` contenant:
         * rapport.pdf
         * client_smtp.c
         * Makefile (si vous n'employez pas de Makefile et un IDE, pas besoin d'envoyer les fichiers de l'IDE, toutefois vérifiez que vous compilez bien avec l'option -Wall)
      * indication: ``make clean`` pour ne pas avoir d'exécutable

   * le travail se fera en classe pendant le temps disponible et chez vous

   * *l'enseignant est à disposition* pendant les cours et par e-mail pour vos questions

   * le travail comptera comme une note (comme le 1er labo) et peut se faire par groupe de deux ou trois (pas de groupe de 1)

   * vous devriez
      * vous exercer avec le protocole SMTP comme client avec [telnet](#exemple-session-smtp)
        ou netcat (nc)
      * partir du [corrigé de l'exercice client HTTP](04_Couche_transport/corriges/client-http-corrige.tar.gz)
      * d'abord tester votre client sur un [serveur simulé](#simulation-dun-serveur) à la main par vous (``nc -l -p 2525`` p.ex.)

## Informations

### Outils de développement

Des [outils de développement classiques](07_Couche_application/complements/outils-developpement-classiques.md)

### Exemple session SMTP

|couleur|signification|
|---|---|
|$`\textcolor{black}{\text{noir}}`$|sortie de bash ou de la commande telnet|
|$`\textcolor{blue}{\text{bleu}}`$|entrée au clavier (commandes bash et messages du client SMTP)|
|$`\textcolor{orange}{\text{orange}}`$|messages du serveur|

NB
   * ``smtprel.he-arc.ch`` nécessite le VPN (ou être dans l'Ecole)
   * la ligne vide séparant entête et corps du message est renforcée par un trait horizontal
   * l'écho de ce que l'on tape est généré localement par le mode ligne-à-ligne du client telnet UNIX
   * le protocole TELNET n'est pas utilisé, c'est fonctionnellement similaire à un netcat ou à notre
     client TCP, à part les fins de lignes qui sont envoyées comme CRLF (\r\n, ASCII 13, 10)
   * l'argument de HELO devrait être un nom de machine, mais c'est souvent
     ignoré dans les cas simples.

![exemple session SMTP](07_Couche_application/images/session-smtp.png)

### Exemple de session SMTP avec erreur fatale anti-relaying

```
schaefer@reliant:~$ telnet smtp.alphanet.ch 25
Trying 46.140.72.222...
Connected to smtp.alphanet.ch.
Escape character is '^]'.
220 shakotay.alphanet.ch ESMTP Postfix (Debian/GNU)
HELO toto.alphanet.ch
250 shakotay.alphanet.ch
MAIL FROM: <schaefer@alphanet.ch>
250 2.1.0 Ok
RCPT TO: <marc.schaefer@he-arc.ch>
554 5.7.1 <marc.schaefer@he-arc.ch>: Relay access denied
```

### Exemple de session SMTP avec erreur temporaire greylisting

```
root@ns2 ~ # telnet smtp.alphanet.ch 25
Trying 46.140.72.222...
Connected to smtp.alphanet.ch.
Escape character is '^]'.
220 shakotay.alphanet.ch ESMTP Postfix (Debian/GNU)
HELO abcd
250 shakotay.alphanet.ch
MAIL FROM: <a@alphanet.ch>
250 2.1.0 Ok
RCPT TO: <schaefer@alphanet.ch>
450 4.2.0 <schaefer@alphanet.ch>: Recipient address rejected: Greylisted, see http://postgrey.schweikert.ch/help/alphanet.ch.html
```

A partir de là, fermer proprement la connexion, attendre un peu, et recommencer depuis le début.

### Simulation d'un serveur

Le plus simple est d'écouter sur un port non privilégié (pas besoin des droits root): ``nc -l -p 2525`` et d'indiquer à votre client SMTP d'utiliser le port 2525 (dernier argument optionnel).

Pour utiliser le port 25, il faut les droits root (sudo) et il ne faut pas qu'il y ait déjà un service écoutant sur ce port: il est très probable que votre Debian ait un daemon exim4 déjà sur ce port.

### Rappel: DNS

Vous pouvez obtenir le MX (mail exchanger SMTP) de la HE-Arc avec
``host -t mx he-arc.ch`` ou lancer ``nslookup`` et entrer:
```
> set querytype=mx
> he-arc.ch.
```

Il n'est pas nécessaire d'implémenter cela dans votre logiciel.

### Recommandations détaillées

   * repartez du corrigé du client HTTP que nous avons fait ensemble, en particulier ne réimplémentez pas la requête DNS (getaddrinfo(3)), la connection TCP (connect(2)), ou la promotion du socket en FILE * (fdopen(3)): utilisez simplement tcp_connect() en y passant les arguments correspondants de la ligne de commande: pour rappel, la conversion de la chaîne (char *) d'argument correspondant au numéro de port en entier (int) n'est pas nécessaire, tout est fait par la fonction tcp_connect()
   * utilisez fprintf(3), fgets(3) et fputs(3) pour travailler en mode ligne à ligne (NVT) -- n'utilisez pas read(2) ou write(2) qui ont plusieurs problèmes dont le fait de ne pas savoir ce qu'est une ligne NVT
   * vous pouvez, comme nous l'avons fait dans le client HTTP, supposer qu'une ligne est limitée à p.ex. 1000 octets: de toute façon fgets() est sécurisé à ce point de vue vu qu'on y passe la taille du tampon
   * il ne devrait pas être nécessaire d'allouer de la mémoire avec malloc/calloc/strdup ou de copier des tampons avec str[n]cpy ou memcpy: si vous avez l'impression que vous en avez besoin, réfléchissez mieux à simplifier votre code
   * évitez les répétitions de code: si vous devez lire quelque chose du serveur dans CHAQUE ETAT, alors il doit être possible de sortir cette opération en-dehors du switch des états et donc d'avoir une seule fois le code correspondant

## Références

   * protocole SMTP
      * [RFC-821 (standard original)]([http://james.apache.org/server/rfclist/smtp/rfc0821.txt), [RFC-2821 (proposed standard)](http://james.apache.org/server/rfclist/smtp/rfc2821.txt), [RFC-5321 (draft standard)](https://tools.ietf.org/html/rfc5321)
      * http://www.commentcamarche.net/contents/536-les-protocoles-de-messagerie-smtp-pop3-et-imap4#le-protocole-smtp

   * rappels sur le C
      * [parser la ligne de commande](http://www.codingunit.com/c-tutorial-command-line-parameter-parsing) (gérer des arguments à un programme en C; mais on peut faire plus simple et utiliser ici directement argc et argv)
      * [lire des fichiers textes en C](https://www.rocq.inria.fr/secret/Anne.Canteaut/COURS_C/chapitre6.html), voir aussi https://openclassrooms.com/courses/apprenez-a-programmer-en-c/lire-et-ecrire-dans-des-fichiers

   * anti-spam, p.ex.
      * https://fr.wikipedia.org/wiki/Greylisting
      * https://fr.wikipedia.org/wiki/DMARC
      * http://fr.wikipedia.org/wiki/Sender_Policy_Framework

<!--

## Améliorations 2022-2023

   * insister conditions de rendu, ou imprimer ce document?

   * avancés, sécurité
      * évaluer la sécurité de la communication entre client et serveur SMTP dans notre cas (confidentialité, intégrité)
      * y-a-t-il une injection possible d'entêtes du mail?  que faire?
      * y-a-t-il une injection possible de commande du protocole SMTP en raison du corps de l'e-mail?  que faire?
-->
