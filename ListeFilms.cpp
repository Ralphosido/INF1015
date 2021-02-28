#pragma region "Includes"//{
#define _CRT_SECURE_NO_WARNINGS // On permet d'utiliser les fonctions de copies de chaînes qui sont considérées non sécuritaires.

#include "ListeFilms.hpp"      // Structures de données pour la collection de films en mémoire.

#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <algorithm>

#include "cppitertools/range.hpp"
#include "gsl/span"

#include "bibliotheque_cours.hpp"
#include "verification_allocation.hpp" // Nos fonctions pour le rapport de fuites de mémoire.
#include "debogage_memoire.hpp"        // Ajout des numéros de ligne des "new" dans le rapport de fuites.  Doit être après les include du système, qui peuvent utiliser des "placement new" (non supporté par notre ajout de numéros de lignes).

using namespace std;
using namespace iter;
using namespace gsl;

#pragma endregion//}

typedef uint8_t UInt8;
typedef uint16_t UInt16;

#pragma region "Fonctions de base pour lire le fichier binaire"//{

UInt8 lireUint8(istream& fichier)
{
	UInt8 valeur = 0;
	fichier.read((char*)&valeur, sizeof(valeur));
	return valeur;
}
UInt16 lireUint16(istream& fichier)
{
	UInt16 valeur = 0;
	fichier.read((char*)&valeur, sizeof(valeur));
	return valeur;
}
string lireString(istream& fichier)
{
	string texte;
	texte.resize(lireUint16(fichier));
	fichier.read((char*)&texte[0], streamsize(sizeof(texte[0])) * texte.length());
	return texte;
}

#pragma endregion//}

void detruireFilm(Film* film);
void afficherFilm(const Film& film);

// Constructeur par défaut

// test git

//testgit 4

//testgit5

//testgit6


ListeFilms::ListeFilms()
{
	capacite_ = 0;
	nElements_ = 0;
}

// Constructeur avec paramètres

ListeFilms::ListeFilms(string nomFichier) 
{
	ifstream fichier(nomFichier, ios::binary);
	fichier.exceptions(ios::failbit);

	int nElements = lireUint16(fichier);

	//TODO: Créer une liste de films vide.
	capacite_ = 0;
	nElements_ = 0;

	for (auto i : range(nElements)) {
		ajouterFilm(lireFilm(fichier)); //TODO: Ajouter le film à la liste.
	}

}

// getter et setter

int ListeFilms::getnElements()
{
	return nElements_;
}

Film** ListeFilms::getElements()
{
	return elements_;
}

void ListeFilms::setElements(Film** nouveauElements)
{
	elements_ = nouveauElements;
}

// méthodes de la classe

//TODO: Une fonction pour ajouter un Film à une ListeFilms, le film existant déjà; on veut uniquement ajouter le pointeur vers le film existant.  Cette fonction doit doubler la taille du tableau alloué, avec au minimum un élément, dans le cas où la capacité est insuffisante pour ajouter l'élément.  Il faut alors allouer un nouveau tableau plus grand, copier ce qu'il y avait dans l'ancien, et éliminer l'ancien trop petit.  Cette fonction ne doit copier aucun Film ni Acteur, elle doit copier uniquement des pointeurs.

void ListeFilms::ajouterFilm(Film* film)
{

	if (capacite_ == 0)
	{
		int nouvelleCapacite = 2;
		elements_ = new Film*[nouvelleCapacite];
		capacite_ = nouvelleCapacite;
	}
	if (nElements_ == capacite_)
	{
		int nouvelleCapacite = capacite_ * 2;
		Film** ptrTableauFilm = new Film*[nouvelleCapacite];

		for (auto i : range(nElements_))
		{
			ptrTableauFilm[i] = elements_[i];
		}

		delete[] elements_;
		elements_ = ptrTableauFilm;
		capacite_ = nouvelleCapacite;
	}

	elements_[nElements_] = film;
	nElements_++;
}


//TODO: Une fonction pour enlever un Film d'une ListeFilms (enlever le pointeur) sans effacer le film; la fonction prenant en paramètre un pointeur vers le film à enlever.  L'ordre des films dans la liste n'a pas à être conservé.

void ListeFilms::enleverFilm(Film* film)
{
	Film** ptrTableauFilm = new Film*[capacite_];

	int compteur = 0;

	for (auto i : range(nElements_))
	{
		if (elements_[i] != film)
		{
			ptrTableauFilm[compteur] = elements_[i];
			compteur++;
		}
	}

	delete[] elements_;
	elements_ = ptrTableauFilm;
	nElements_--;
}


//TODO: Une fonction pour trouver un Acteur par son nom dans une ListeFilms, qui retourne un pointeur vers l'acteur, ou nullptr si l'acteur n'est pas trouvé.  Devrait utiliser span.
span<Acteur*> spanListeActeurs(const ListeActeurs& liste) { return span(liste.elements, liste.nElements); }

Acteur* ListeFilms::trouverActeur(string& nomActeur)
{
	Acteur* ptrActeur = nullptr;


	for (const Film* film : enSpan())
	{
		for (Acteur* acteur : spanListeActeurs(film->acteurs))
		{
			if (acteur->nom == nomActeur)
				ptrActeur = acteur;
		}
	}

	return ptrActeur;
}

//TODO: Compléter les fonctions pour lire le fichier et créer/allouer une ListeFilms.  La ListeFilms devra être passée entre les fonctions, pour vérifier l'existence d'un Acteur avant de l'allouer à nouveau (cherché par nom en utilisant la fonction ci-dessus).
Acteur* ListeFilms::lireActeur(istream& fichier)
{
	Acteur acteur = {};
	acteur.nom = lireString(fichier);
	acteur.anneeNaissance = lireUint16(fichier);
	acteur.sexe = lireUint8(fichier);

	Acteur* ptrActeur = trouverActeur(acteur.nom);

	if (ptrActeur == nullptr)
	{
		Acteur* ptrNouveauActeur = new Acteur;
		acteur.joueDans = ListeFilms();

		(*ptrNouveauActeur) = acteur;
		cout << "Création Acteur " << acteur.nom << endl;
		return ptrNouveauActeur;
	}
	else
		return ptrActeur; //TODO: Retourner un pointeur soit vers un acteur existant ou un nouvel acteur ayant les bonnes informations, selon si l'acteur existait déjà.  Pour fins de débogage, affichez les noms des acteurs crées; vous ne devriez pas voir le même nom d'acteur affiché deux fois pour la création.
}

Film* ListeFilms::lireFilm(istream& fichier)
{
	Film film = {};
	film.titre = lireString(fichier);
	film.realisateur = lireString(fichier);
	film.anneeSortie = lireUint16(fichier);
	film.recette = lireUint16(fichier);
	film.acteurs.nElements = lireUint8(fichier);  //NOTE: Vous avez le droit d'allouer d'un coup le tableau pour les acteurs, sans faire de réallocation comme pour ListeFilms.  Vous pouvez aussi copier-coller les fonctions d'allocation de ListeFilms ci-dessus dans des nouvelles fonctions et faire un remplacement de Film par Acteur, pour réutiliser cette réallocation.

	Film* ptrFilm = new Film;
	(*ptrFilm) = film;
	film.acteurs.capacite = film.acteurs.nElements;

	film.acteurs.elements = new Acteur*[film.acteurs.nElements];

	for (auto i : range(film.acteurs.nElements)) {
		film.acteurs.elements[i] = lireActeur(fichier); //TODO: Placer l'acteur au bon endroit dans les acteurs du film.
		(*ptrFilm) = film;
		(*film.acteurs.elements[i]).joueDans.ajouterFilm(ptrFilm); //TODO: Ajouter le film à la liste des films dans lesquels l'acteur joue.
	}

	cout << "Création Film " << film.titre << endl;

	return ptrFilm; //TODO: Retourner le pointeur vers le nouveau film.
}

//TODO: Une fonction pour détruire une ListeFilms et tous les films qu'elle contient.

void ListeFilms::detruireListeFilms()
{

	for (auto i : range(nElements_))
	{
		detruireFilm(elements_[i]);
	}

	delete[] elements_;
	elements_ = nullptr;
	capacite_ = nElements_ = 0;
}

void afficherActeur(const Acteur& acteur)
{
	cout << "  " << acteur.nom << ", " << acteur.anneeNaissance << " " << acteur.sexe << endl;
}


void afficherListeFilms(const ListeFilms& listeFilms)
{
	//TODO: Utiliser des caractères Unicode pour définir la ligne de séparation (différente des autres lignes de séparations dans ce progamme).
	static const string ligneDeSeparation = "=========================================";
	cout << ligneDeSeparation << endl;
	//TODO: Changer le for pour utiliser un span.

	/*for (auto i : range(nElements_)) {
		//TODO: Afficher le film.
		afficherFilm(*(elements_[i]));
		cout << ligneDeSeparation << endl;
	}*/

	for (const Film* film : listeFilms.enSpan()) {
		afficherFilm(*film);
		cout << ligneDeSeparation;
	}
}

void ListeFilms::afficherFilmographieActeur(string& nomActeur)
{
	//TODO: Utiliser votre fonction pour trouver l'acteur (au lieu de le mettre à nullptr).
	const Acteur* acteur = trouverActeur(nomActeur);

	if (acteur == nullptr)
		cout << "Aucun acteur de ce nom" << endl;
	else
	{
		ListeFilms listeFilms;
		listeFilms = (*acteur).joueDans;
		afficherListeFilms(listeFilms);
	}
}

// Fonctions qui ne font pas partie de la classe ListeFilms

//TODO: Une fonction pour détruire un film (relâcher toute la mémoire associée à ce film, et les acteurs qui ne jouent plus dans aucun films de la collection).  Noter qu'il faut enleve le film détruit des films dans lesquels jouent les acteurs.  Pour fins de débogage, affichez les noms des acteurs lors de leur destruction.

void detruireFilm(Film* film)
{

	for (auto i : range((*film).acteurs.nElements))
	{
		(*(*film).acteurs.elements[i]).joueDans.enleverFilm(film);
		if ((*(*film).acteurs.elements[i]).joueDans.getnElements() == 0)
		{
			cout << "Destruction Acteur " << (*(*film).acteurs.elements[i]).nom << endl;
			delete[](*(*film).acteurs.elements[i]).joueDans.getElements();
			(*(*film).acteurs.elements[i]).joueDans.setElements(nullptr);
			delete ((*film).acteurs.elements[i]);
			(*film).acteurs.elements[i] = nullptr;
		}
	}

	cout << "Destruction Film " << (*film).titre << endl;
	delete[](*film).acteurs.elements;
	(*film).acteurs.elements = nullptr;
	delete film;
	film = nullptr;
}

//TODO: Une fonction pour afficher un film avec tous ces acteurs (en utilisant la fonction afficherActeur ci-dessus).
void afficherFilm(const Film& film)
{
	cout << "Titre: " << film.titre << endl;
	cout << "  " << "Réalisateur: " << film.realisateur << " Année: " << film.anneeSortie << " " << endl;
	cout << "  " << "Recette: " << film.recette << "M$" << endl;
	cout << "Acteurs: " << endl;

	for (auto i : range(film.acteurs.nElements))
	{
		afficherActeur(*(film.acteurs.elements[i]));
	}
}

int main()
{
#ifdef VERIFICATION_ALLOCATION_INCLUS
	bibliotheque_cours::VerifierFuitesAllocations verifierFuitesAllocations;
#endif
	bibliotheque_cours::activerCouleursAnsi();  // Permet sous Windows les "ANSI escape code" pour changer de couleurs https://en.wikipedia.org/wiki/ANSI_escape_code ; les consoles Linux/Mac les supportent normalement par défaut.

	//int* fuite = new int; //TODO: Enlever cette ligne après avoir vérifié qu'il y a bien un "Fuite detectee" de "4 octets" affiché à la fin de l'exécution, qui réfère à cette ligne du programme.

	static const string ligneDeSeparation = "\n\033[35m════════════════════════════════════════\033[0m\n";

	//TODO: Chaque TODO dans cette fonction devrait se faire en 1 ou 2 lignes, en appelant les fonctions écrites.

	//TODO: La ligne suivante devrait lire le fichier binaire en allouant la mémoire nécessaire.  Devrait afficher les noms de 20 acteurs sans doublons (par l'affichage pour fins de débogage dans votre fonction lireActeur).

	ListeFilms listeFilms = ListeFilms("films.bin");

	cout << ligneDeSeparation << "Le premier film de la liste est:" << endl;
	//TODO: Afficher le premier film de la liste.  Devrait être Alien.

	afficherFilm(*(listeFilms.getElements()[0]));

	cout << ligneDeSeparation << "Les films sont:" << endl;
	//TODO: Afficher la liste des films.  Il devrait y en avoir 7.

	afficherListeFilms(listeFilms);

	//TODO: Modifier l'année de naissance de Benedict Cumberbatch pour être 1976 (elle était 0 dans les données lues du fichier).  Vous ne pouvez pas supposer l'ordre des films et des acteurs dans les listes, il faut y aller par son nom.

	string nom = "Benedict Cumberbatch";
	Acteur* ptrBenedict = listeFilms.trouverActeur(nom);
	(*ptrBenedict).anneeNaissance = 1976;

	cout << ligneDeSeparation << "Liste des films où Benedict Cumberbatch joue sont:" << endl;
	//TODO: Afficher la liste des films où Benedict Cumberbatch joue.  Il devrait y avoir Le Hobbit et Le jeu de l'imitation.

	listeFilms.afficherFilmographieActeur(nom);

	//TODO: Détruire et enlever le premier film de la liste (Alien).  Ceci devrait "automatiquement" (par ce que font vos fonctions) détruire les acteurs Tom Skerritt et John Hurt, mais pas Sigourney Weaver puisqu'elle joue aussi dans Avatar.

	Film* filmADetruire = listeFilms.getElements()[0];
	listeFilms.enleverFilm(filmADetruire);
	detruireFilm(filmADetruire);

	cout << ligneDeSeparation << "Les films sont maintenant:" << endl;
	//TODO: Afficher la liste des films.

	afficherListeFilms(listeFilms);

	//TODO: Faire les appels qui manquent pour avoir 0% de lignes non exécutées dans le programme (aucune ligne rouge dans la couverture de code; c'est normal que les lignes de "new" et "delete" soient jaunes).  Vous avez aussi le droit d'effacer les lignes du programmes qui ne sont pas exécutée, si finalement vous pensez qu'elle ne sont pas utiles.

	string nomActeurInexistant = "Justine";
	listeFilms.afficherFilmographieActeur(nomActeurInexistant);

	//TODO: Détruire tout avant de terminer le programme.  L'objet verifierFuitesAllocations devrait afficher "Aucune fuite detectee." a la sortie du programme; il affichera "Fuite detectee:" avec la liste des blocs, s'il manque des delete.
	listeFilms.detruireListeFilms();
}
