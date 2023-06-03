Cliente Suisse Fiverr

Travail à rendre (rapport et logiciel)
Il y a un code source et un rapport à rendre, par groupe de 2.
Les objectifs du code source à coder et rendre sont:


coder un client SMTP en langage C sous plateforme POSIX (GNU/Linux) qui puisse, à partir
d'informations de la ligne de commande envoyer un mail via
le protocole SMTP, y compris la gestion des erreurs SMTP (fatales ou temporaires, avec notamment la gestion de la
réémission en cas de grey-listing)


les arguments de la ligne de commande sont, dans l'ordre

e-mail expéditeur,
sujet du message (entre guillemets si multi-mots, cf bash)
nom de fichier du corps du message
nom de domaine DNS ou adresse IP du serveur de mail à utiliser
e-mail destinataire
argument optionnel: numéro de port (par défaut 25)



le client doit afficher un retour à l'utilisateur si l'e-mail a bien été
envoyé ou afficher le message d'erreur SMTP dans le cas contraire.


en pratique, le client doit avoir été testé

avec un serveur local simulé par vous avec la commande netcat (nc), y compris erreurs fatales et temporaires (grey-listing)
sur le serveur de l'école smtprel.he-arc.ch avec expéditeur quelconque et votre adresse e-mail de destination
sur le serveur smtp.alphanet.ch avec destinataire votre adresse e-mail HE-Arc (but: observer
l'anti-relaying et informer l'utilisateur de l'erreur fatale)

indication: utiliser le port 587 plutôt que 25 depuis l'Ecole ou Swisscom (firewall/proxy "intelligent")
destinataires: schaefer@alphanet.ch ou info@alphanet.ch (ou test@alphanet.ch pour recevoir au plus 1 réponse par jour si votre adresse From: est valable et que le sujet du message est test)
(but: observer le grey-listing, attendre un peu et réessayer)
en cas d'erreur 5xx systématique, activez le VPN (interne; votre adresse dynamique xDSL est peut-être blacklistée)


les tests sont documentés dans le rapport (minimum: copie d'écran)



#PhasesDeveloppement

proposition de phases de développement

1: réflexion sur une architecture d'automate du client SMTP

chacune des phases du protocole (connexion, envoi du HELO, envoi du MAIL FROM, envoi du RCPT TO, envoi du DATA, envoi des données, fin) peut être considérée comme un état d'un automate: dessinez ces états
les transitions entre les phases sont basées sur des conditions: des événements provenant du serveur, voire de la couche 4 (p.ex. après connexion, le
220 du serveur permet d'envoyer le HELO, etc): indiquez les conditions des transitions et ce que le client envoie en réponse (p.ex. sous la forme condition/réaction ou plus généralement: attendu/envoyé)
expliquez ce que vous faites si le code attendu n'est pas obtenu et où vous le faites pour éviter la duplication de code (D.R.Y.)

cas spécial: code 4xx
autres cas (erreurs)


proposez une implémentation C de l'automate client SMTP, basée sur switch variable_etat et une boucle "infinie": voir l'exemple d'automate de protocole réalisé en C



2: envoi d'un mail dont le contenu est "hard-codé", en parsant la ligne de commande pour les
autres éléments, en tenant compte du protocole SMTP avec le serveur, mais en ignorant les erreurs; en
affichant le déroulé à l'écran
3: lecture d'un fichier spécifié pour le contenu
4: parsing des codes d'erreurs du serveur et plantage avec informations à l'écran si pas attendu
5: implémentation de la fonction de ré-essai pour le grey-listing

en cas de code 4xx ou en cas de réussite, la session SMTP est terminée proprement, la connexion est fermée (avant le réessai éventuel)
