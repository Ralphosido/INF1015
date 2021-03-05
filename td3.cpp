#pragma region "Includes"//{
#define _CRT_SECURE_NO_WARNINGS // On permet d'utiliser les fonctions de copies de chaînes qui sont considérées non sécuritaires.

#include "td3.hpp"      // Structures de données pour la collection de films en mémoire.

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


// Fonction pour ajouter un Film à une ListeFilms.

void ListeFilms::ajouterFilm(Film* film)
{

	if (capacite_ == 0)
	{
		int nouvelleCapacite = 1;
		elements_ = new Film*[nouvelleCapacite];
		capacite_ = nouvelleCapacite;
	}
	if (nElements_ == capacite_)
	{
		int nouvelleCapacite = capacite_ * 2;
		Film** ptrTableauFilm = new Film*[nouvelleCapacite];

		for (int i : range(nElements_))
		{
			ptrTableauFilm[i] = elements_[i];
		}
		delete[] elements_;
		elements_ = ptrTableauFilm;
		capacite_ = nouvelleCapacite;
	}

	elements_[nElements_++] = film;
}

// Fonction pour enlever un Film d'une ListeFilms (enlever le pointeur) sans effacer le film; la fonction prenant en paramètre un pointeur vers le film à enlever.  L'ordre des films dans la liste n'a pas à être conservé.

span<Film*> ListeFilms::enSpan() const { return span(elements_, nElements_); }

void ListeFilms::enleverFilm(const Film* film)
{
	for (Film*& element : enSpan()) {  // Doit être une référence au pointeur pour pouvoir le modifier.
		if (element == film) {
			if (nElements_ > 1)
				element = elements_[nElements_ - 1];
			nElements_--;
			return;
		}
	}
}


//TODO: Une fonction pour trouver un Acteur par son nom dans une ListeFilms, qui retourne un pointeur vers l'acteur, ou nullptr si l'acteur n'est pas trouvé.  Devrait utiliser span.

shared_ptr<Acteur> ListeFilms::trouverActeur(const string& nomActeur) const
{
	for (const Film* film : enSpan())
	{
		for (shared_ptr<Acteur> acteur : film->acteurs.enSpan())
		{
			if (acteur->nom == nomActeur)
				return acteur;
		}
	}
	return nullptr;
}

//TODO: Compléter les fonctions pour lire le fichier et créer/allouer une ListeFilms.  La ListeFilms devra être passée entre les fonctions, pour vérifier l'existence d'un Acteur avant de l'allouer à nouveau (cherché par nom en utilisant la fonction ci-dessus).

shared_ptr<Acteur> lireActeur(istream& fichier, ListeFilms& listeFilms)
{
	Acteur acteur = {};
	acteur.nom = lireString(fichier);
	acteur.anneeNaissance = lireUint16(fichier);
	acteur.sexe = lireUint8(fichier);

	shared_ptr<Acteur> ptrActeur = listeFilms.trouverActeur(acteur.nom);

	if (ptrActeur == nullptr)
	{
		cout << "Création Acteur " << acteur.nom << endl;
		//return new Acteur(acteur); 
		return make_shared<Acteur>(acteur);
	}
	else
		return ptrActeur; //TODO: Retourner un pointeur soit vers un acteur existant ou un nouvel acteur ayant les bonnes informations, selon si l'acteur existait déjà.  Pour fins de débogage, affichez les noms des acteurs crées; vous ne devriez pas voir le même nom d'acteur affiché deux fois pour la création.
}


Film* lireFilm(istream& fichier, ListeFilms& listeFilms)
{
	Film* film = new Film;
	film->titre = lireString(fichier);
	film->realisateur = lireString(fichier);
	film->anneeSortie = lireUint16(fichier);
	film->recette = lireUint16(fichier);
	int nElements = lireUint8(fichier);

	ListeActeurs acteurs(nElements);
	film->acteurs = move(acteurs);
	//film->acteurs.setnElements(nElements);
	//film->acteurs.setCapacite(nElements);

	cout << "Création Film " << film->titre << endl;
	for (shared_ptr<Acteur>& acteur : film->acteurs.enSpan()) {
		acteur = lireActeur(fichier, listeFilms);
		//acteur->joueDans.ajouterFilm(film);
	}

	

	/*ListeActeurs acteurs(nElements);
	//film->acteurs = move(acteurs);
	film->acteurs.setnElements(nElements);
	film->acteurs.setCapacite(nElements);

	cout << "Création Film " << film->titre << endl;
	for (shared_ptr<Acteur>& acteur : acteurs.enSpan()) {
		acteur = lireActeur(fichier, listeFilms);
		//acteur->joueDans.ajouterFilm(film);
	}

	film->acteurs.setCapacite(nElements);
	film->acteurs.setElements(acteurs.getElements());
	//setElement(shared_ptr<T> nouveauElement, int pos)*/
	return film;
	 
}

ListeFilms creerListe(string nomFichier)
{
	ifstream fichier(nomFichier, ios::binary);
	fichier.exceptions(ios::failbit);

	int nElements = lireUint16(fichier);

	ListeFilms listeFilms;
	for ([[maybe_unused]] int i : range(nElements)) { //NOTE: On ne peut pas faire un span simple avec ListeFilms::enSpan car la liste est vide et on ajoute des éléments à mesure.
		listeFilms.ajouterFilm(lireFilm(fichier, listeFilms));
	}
	return listeFilms;
}

// Fonctions pour détruire un film
/*void detruireActeur(shared_ptr<Acteur> acteur)
{
	cout << "Destruction Acteur " << acteur->nom << endl;
	//acteur->joueDans.detruire();
	//delete acteur;
}*/

/*bool joueEncore(const shared_ptr<Acteur> acteur)
{
	return acteur.use_count() != 0;
}*/

void detruireFilm(Film* film)
{
	for (shared_ptr<Acteur> acteur : film->acteurs.enSpan())
	{
		//acteur->joueDans.enleverFilm(film);
		//cout << acteur.use_count() << acteur->nom << endl;
		//if (acteur.use_count() == 0)
			//cout << "Destruction Acteur " << acteur->nom << endl;
	}
	cout << "Destruction Film " << film->titre << endl;
	delete film;
}

 
//TODO: Une fonction pour détruire une ListeFilms et tous les films qu'elle contient.

void ListeFilms::detruire(bool possedeLesFilms)
{
	if (possedeLesFilms)
		for (Film* film : enSpan())
			detruireFilm(film);
	delete[] elements_;
}


void afficherActeur(const Acteur& acteur)
{
	cout << "  " << acteur.nom << ", " << acteur.anneeNaissance << " " << acteur.sexe << endl;
}

//TODO: Une fonction pour afficher un film avec tous ces acteurs (en utilisant la fonction afficherActeur ci-dessus).



ostream& operator<<(ostream& o, const Film& film)
{
	o << "Titre: " << film.titre << " " << "Réalisateur: " << film.realisateur << " Année: "
		<< film.anneeSortie << " " << "Recette: " << film.recette << "M$" << "Acteurs: " << " ";
	for (auto&& acteur : film.acteurs.enSpan())
	{
		afficherActeur(*acteur);
	}

};

/*void afficherFilm(const Film& film)
{
	cout << "Titre: " << film.titre << endl;
	cout << "  " << "Réalisateur: " << film.realisateur << " Année: " << film.anneeSortie << " " << endl;
	cout << "  " << "Recette: " << film.recette << "M$" << endl;
	cout << "Acteurs: " << endl;

	for (const shared_ptr<Acteur> acteur : film.acteurs.enSpan())
	{
		afficherActeur(*acteur);
	}
}}*/

/*void afficherListeFilms(const ListeFilms& listeFilms)
{
	static const string ligneDeSeparation = "=========================================";
	cout << ligneDeSeparation << endl;

	for (const Film* film : listeFilms.enSpan()) {
		afficherFilm(*film);
		cout << ligneDeSeparation;
	}
}*/


/*void afficherFilmographieActeur(const ListeFilms& listeFilms, const string& nomActeur)
{
	const Acteur* acteur = listeFilms.trouverActeur(nomActeur);
	if (acteur == nullptr)
		cout << "Aucun acteur de ce nom" << endl;
	else
	{
		afficherListeFilms(acteur->joueDans);
	}
}*/


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

	ListeFilms listeFilms = creerListe("films.bin");

	cout << ligneDeSeparation << "Le premier film de la liste est:" << endl;
	//TODO: Afficher le premier film de la liste.  Devrait être Alien.

	afficherFilm(*listeFilms.enSpan()[0]);

	cout << ligneDeSeparation << "Les films sont:" << endl;
	//TODO: Afficher la liste des films.  Il devrait y en avoir 7.

	afficherListeFilms(listeFilms);

	//TODO: Modifier l'année de naissance de Benedict Cumberbatch pour être 1976 (elle était 0 dans les données lues du fichier).  Vous ne pouvez pas supposer l'ordre des films et des acteurs dans les listes, il faut y aller par son nom.

	listeFilms.trouverActeur("Benedict Cumberbatch")->anneeNaissance = 1976;
	

	cout << ligneDeSeparation << "Liste des films où Benedict Cumberbatch joue sont:" << endl;
	//TODO: Afficher la liste des films où Benedict Cumberbatch joue.  Il devrait y avoir Le Hobbit et Le jeu de l'imitation.

	//afficherFilmographieActeur(listeFilms, "Benedict Cumberbatch");

	//TODO: Détruire et enlever le premier film de la liste (Alien).  Ceci devrait "automatiquement" (par ce que font vos fonctions) détruire les acteurs Tom Skerritt et John Hurt, mais pas Sigourney Weaver puisqu'elle joue aussi dans Avatar.

	detruireFilm(listeFilms.enSpan()[0]);
	listeFilms.enleverFilm(listeFilms.enSpan()[0]);


	cout << ligneDeSeparation << "Les films sont maintenant:" << endl;
	//TODO: Afficher la liste des films.

	afficherListeFilms(listeFilms);

	//TODO: Faire les appels qui manquent pour avoir 0% de lignes non exécutées dans le programme (aucune ligne rouge dans la couverture de code; c'est normal que les lignes de "new" et "delete" soient jaunes).  Vous avez aussi le droit d'effacer les lignes du programmes qui ne sont pas exécutée, si finalement vous pensez qu'elle ne sont pas utiles.

	listeFilms.enleverFilm(nullptr); // Enlever un film qui n'est pas dans la liste (clairement que nullptr n'y est pas).
	//afficherFilmographieActeur(listeFilms, "Justine"); 

	//TODO: Détruire tout avant de terminer le programme.  L'objet verifierFuitesAllocations devrait afficher "Aucune fuite detectee." a la sortie du programme; il affichera "Fuite detectee:" avec la liste des blocs, s'il manque des delete.
	listeFilms.detruire(true);

	// Code pour listeTextes (chapitre 9.3, 9.4 et 9.5)
	Liste<string> listeTextes(2);

	shared_ptr<string> str1 = make_shared<string>("Premier string");
	shared_ptr<string> str2 = make_shared<string>("Deuxième string");
	listeTextes.setElement(str1, 0);
	listeTextes.setElement(str2, 1);

	Liste<string> listeTextes2 = listeTextes;

	shared_ptr<string> str3 = make_shared<string>("nouveau string");
	listeTextes.setElement(str3, 0); 
	*str2 = "string modifié";

	cout << ligneDeSeparation << "ListeTextes :" << endl;

	for (const shared_ptr<string> str : listeTextes.enSpan())
	{
		cout << *str << endl;
	}

	cout << ligneDeSeparation << "ListeTextes2 :" << endl;

	for (const shared_ptr<string> str : listeTextes2.enSpan())
	{
		cout << *str << endl;
	}
	
}
