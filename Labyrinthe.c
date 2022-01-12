#include <MLV/MLV_all.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "structures.h"
#ifdef _WIN32
#include <windows.h> // pour windows
#define WINDOWS
#else
#include <unistd.h> // pour linux
#endif
#define LARGEUR 920
#define HAUTEUR 720

typedef struct coordonnees_t {
    int x;
    int y;
} coordonnees_t;

typedef struct case_t {
    int murEst;
    int murSud;
    coordonnees_t pere;
    int rang;
} case_t;

typedef struct labyrinthe_t {
    coordonnees_t taille;
    case_t **cases;
} labyrinthe_t;

typedef struct mur {
	int x;
	int y;
	int murEst;
} Mur;

typedef struct noeud { // La structure pour l'algorithme A*
	int cout_g, cout_h, cout_f;
	int x, y;
	struct noeud *parent;
} Noeud, *Arbre;

int initialiser(labyrinthe_t *laby) {
    // Initialise le labyrinthe, 1 en cas de réussite 0 sinon
    int n = laby->taille.x;
    int m = laby->taille.y;
    laby->cases = calloc(m, sizeof(case_t));
    if(laby->cases == NULL) {
        return 0;
    }
    int i, j;
    for(i = 0; i < m; i++) {
        laby->cases[i] = calloc(n, sizeof(case_t));
        if(laby->cases[i] == NULL) {
            return 0;
        }
        for(j = 0; j < n; j++) {
            laby->cases[i][j].murEst = 1;
            laby->cases[i][j].murSud = 1;
            coordonnees_t coord;
            coord.x = j;
            coord.y = i;
            laby->cases[i][j].pere = coord;
            laby->cases[i][j].rang = 0;
        }
    }
    laby->cases[m - 1][n - 1].murEst = 0;
    return 1;
}

int abs(int x) {
	if(x < 0) {
		return -x;
	}
	return x;
}

int coordonneesEgales(coordonnees_t c1, coordonnees_t c2) {
    // Retourne 1 si les deux coordonnees sont égales
    return c1.x == c2.x && c1.y == c2.y;
}

coordonnees_t trouver(labyrinthe_t laby, int x, int y) {
    // Retourne à quel ensemble appartient la coordonnée (x, y)
    // C'est fonction est aussi une fonction trouve et compresse
    coordonnees_t coord = laby.cases[y][x].pere;
    coordonnees_t pere;
    coordonnees_t temp; // Qui permet de garder le père de la coordonnée actuelle
    int rg = 0;
    while(!coordonneesEgales(coord, laby.cases[coord.y][coord.x].pere)) {
        coord = laby.cases[coord.y][coord.x].pere;
        rg++;
    }
    pere = coord;
    // On passe à la compression
    if(rg > 0) {
		temp = laby.cases[y][x].pere;
		laby.cases[y][x].pere = pere;
		while(!coordonneesEgales(laby.cases[temp.y][temp.x].pere, pere)) {
			coord = temp;
			temp = laby.cases[temp.y][temp.x].pere;
			laby.cases[coord.y][coord.x].pere = pere;
		}
	}
    return pere;
}

int memeClasse(labyrinthe_t laby, int x1, int y1, int x2, int y2) {
    // Retourne 1 si les deux coordonnées (x1, y1), (x2, y2) sont dans la meme classe
    return coordonneesEgales(trouver(laby, x1, y1), trouver(laby, x2, y2));
}

int calculerRang(labyrinthe_t laby, int x, int y) {
    int rang = 0;
    coordonnees_t coord;
    coord.x = x;
    coord.y = y;
    while(!coordonneesEgales(coord, laby.cases[coord.y][coord.x].pere)) {
        coord = laby.cases[coord.y][coord.x].pere;
        rang++;
    }
    return rang;
}

void relierCases(labyrinthe_t *laby, coordonnees_t point1, coordonnees_t rac1, coordonnees_t point2, coordonnees_t rac2) {
	// On utilise la fusion par rang
	int rg;
	if(coordonneesEgales(rac1, point1) && laby->cases[rac1.y][rac1.x].rang == 0) { // le point n'est relié à personne
		laby->cases[rac1.y][rac1.x].pere = point2;
        rg = calculerRang(*laby, rac1.x, rac1.y);
		if(rg > laby->cases[rac2.y][rac2.x].rang) {
			laby->cases[rac2.y][rac2.x].rang = rg;
		}
	}
	else if(coordonneesEgales(rac2, point2) && laby->cases[rac2.y][rac2.x].rang == 0) { // le deuxième point n'est relié à personne
		laby->cases[rac2.y][rac2.x].pere = point1;
        rg = calculerRang(*laby, rac2.x, rac2.y);
		if(rg > laby->cases[rac1.y][rac1.x].rang) {
			laby->cases[rac1.y][rac1.x].rang = rg;
		}
	}
	else if(laby->cases[rac2.y][rac2.x].rang == laby->cases[rac1.y][rac1.x].rang) { // les deux arbres ont la même hauteur mais pas la même racine
		laby->cases[rac2.y][rac2.x].pere = rac1;
		laby->cases[rac1.y][rac1.x].rang++;
	}
	else if(laby->cases[rac2.y][rac2.x].rang > laby->cases[rac1.y][rac1.x].rang) { // l'arbre 2 est plus grand que le premier
		laby->cases[rac1.y][rac1.x].pere = rac2;
	}
	else { // l'arbre 1 est plus grand que le deuxième
		laby->cases[rac2.y][rac2.x].pere = rac1;
	}
}

int separation(labyrinthe_t laby, coordonnees_t point1, coordonnees_t point2) {
	// Retourne 1 s'il y a une séparation de mur entre le point 1 et le point 2, 0 sinon
	if(point2.x > point1.x) {
		if(laby.cases[point1.y][point1.x].murEst) {
			return 1;
		}
		return 0;
	}
	if(point1.x > point2.x) {
		if(laby.cases[point2.y][point2.x].murEst) {
			return 1;
		}
		return 0;
	}
	if(point2.y > point1.y) {
		if(laby.cases[point1.y][point1.x].murSud) {
			return 1;
		}
		return 0;
	}
	if(point1.y > point2.y) {
		if(laby.cases[point2.y][point2.x].murSud) {
			return 1;
		}
	}
	return 0;
}

void supprimerMur(labyrinthe_t *laby, int unique, int optim, Mur *murs, int indice) {
    int x;
    int y;
    int tailleX = laby->taille.x;
    int tailleY = laby->taille.y;
    coordonnees_t point1; // coordonnées de notre premier point généré
    coordonnees_t rac1; // racine de notre point aléatoire
    coordonnees_t point2; // coordonnées de notre point qui va permettre la liaison liaison des deux racines
    coordonnees_t rac2; // racine de notre point qui va se lier au précédent
    int mur_coord; // les coordonnées du mur
    int alea = rand() % 2; // qui va permettre de déterminer les coordonnées du point
    if(!optim) {
		do {
			x = rand() % tailleX;
			y = rand() % tailleY;
		} while(x == tailleX - 1 && y == tailleY - 1); // pour éviter en bas à droite
	}
	else {
		x = murs[indice].x;
		y = murs[indice].y;
		point2.x = x;
		point2.y = y;
		if(murs[indice].murEst) {
			point2.y++;
		}
		else {
			point2.x++;
		}
	}
    point1.x = x;
    point1.y = y;
    if(!optim) {
		point2.x = x;
		point2.y = y;
		if(x == tailleX - 1) {
			if(y != tailleY - 1) {
				point2.y++;
			}
		}
		else if(y == tailleY - 1) {
			point2.x++;
		}
		else {
			if(alea)  {
				point2.x++;
			}
			else {
				point2.y++;
			}
		}
	}
    rac1 = trouver(*laby, x, y);
    rac2 = trouver(*laby, point2.x, point2.y);
    if(unique && !memeClasse(*laby, x, y, point2.x, point2.y)) { // si on veut un chemin unique, alors on ne doit pas supprimer des murs qui séparent deux cases de la même classe
		if(point2.x > x) {
			laby->cases[y][x].murEst = 0;
		}
		else {
			laby->cases[y][x].murSud = 0;
		}
		relierCases(laby, point1, rac1, point2, rac2);
	}
	else if(!unique && separation(*laby, point1, point2)) {
		if(point2.x > x) {
			laby->cases[y][x].murEst = 0;
		}
		else {
			laby->cases[y][x].murSud = 0;
		}
		if(!coordonneesEgales(rac1, rac2)) {
			relierCases(laby, point1, rac1, point2, rac2);
		}
	}
}

void affichageTexte(labyrinthe_t laby) {
	// Affiche le labyrinthe dans la console
    printf("Affichage texte du labyrinthe\n");
    int y, x, i;
    int tailleX = laby.taille.x;
    int tailleY = laby.taille.y;
    for(y = 0; y < tailleY; y++) {
        if(y == 0) {
            for(x = 0; x < tailleX; x++) {
				if(x - 1 < 0) {
					printf("%s", intersections[0][0][0][1]);
				}
				else {
					printf("%s", intersections[laby.cases[0][x - 1].murEst][0][1][1]);
				}
				for(i = 0; i < 2; i++) {
					printf("%s", intersections[0][0][1][1]);
				}
            }
            printf("%s\n ", intersections[1][0][1][0]);
        }
        else {
            printf("%s", intersections[1][1][0][0]);
        }
        for(x = 0; x < tailleX; x++) {
            printf("  ");
			printf("%s", intersections[laby.cases[y][x].murEst][laby.cases[y][x].murEst][0][0]);
        }
        printf("\n");
        for(x = 0; x < tailleX; x++) {
			if(x == 0) {
				printf("%s", intersections[y == tailleY - 1 ? 0 : 1][y == 0 ? 0 : 1][0][laby.cases[y][x].murSud]);
			}
			else if(y == tailleY - 1) {
				printf("%s", intersections[0][laby.cases[y][x - 1].murEst][1][1]);
			}
			else {
				printf("%s", intersections[laby.cases[y + 1][x - 1].murEst][laby.cases[y][x - 1].murEst][laby.cases[y][x - 1].murSud][laby.cases[y][x].murSud]);
			}
			for(i = 0; i < 2; i++) {
				printf("%s", intersections[0][0][laby.cases[y][x].murSud][laby.cases[y][x].murSud]);
			}
        }
        printf("%s\n", intersections[y < tailleY - 2 ? 1 : 0][y == tailleY - 1 ? 0 : 1][laby.cases[y][x - 1].murSud][0]);
    }
}

int minimum(int a, int b) {
	if(a < b) {
		return a;
	}
	return b;
}

int calculerDistance(coordonnees_t c1, coordonnees_t c2) {
	return abs(c1.x - c2.x) * abs(c1.x - c2.x) + abs(c1.y - c2.y) * abs(c1.y - c2.y);
}

void cheminPlusCourt(labyrinthe_t laby, coordonnees_t *chemin, int *taille) {
	// Algorithme A* itératif
	int i, j;
	int **visitees, **couts;
	coordonnees_t actuel;
	coordonnees_t prec;
	coordonnees_t depart;
	coordonnees_t arrivee;
	Arbre *arbre;
	visitees = (int **) calloc(laby.taille.y, sizeof(int *));
	arbre = (Arbre *) calloc(laby.taille.y, sizeof(Arbre));
	coordonnees_t *file = (coordonnees_t *) calloc(11, sizeof(coordonnees_t));
	for(i = 0; i < laby.taille.y; i++) {
		visitees[i] = (int *) calloc(laby.taille.x, sizeof(int));
		arbre[i] = (Arbre) calloc(laby.taille.x, sizeof(Noeud));
		for(j = 0; j < laby.taille.x; j++) {
			arbre[i][j].parent = NULL;
			arbre[i][j].x = j;
			arbre[i][j].y = i;
		}
	}
	actuel.x = 0;
	actuel.y = 0;
	depart.x = 0;
	depart.y = 0;
	arrivee.x = laby.taille.x - 1;
	arrivee.y = laby.taille.y - 1;
	int indice = 0, ajout = 0;
	int fils;
	arbre[0][0].parent = &(arbre[0][0]);
	while(!coordonneesEgales(actuel, arrivee)) { // Tant que nous ne sommes pas arrivés à l'arrivée
		int vois[4][2] = {{actuel.x + 1, actuel.y}, {actuel.x, actuel.y + 1}, {actuel.x - 1, actuel.y}, {actuel.x, actuel.y - 1}}; // Les voisins de notre point actuel
		visitees[actuel.y][actuel.x] = 1;
		fils = 0;
		coordonnees_t prochain;
		int plusProche = 0;
		int distanceMini = 0;
		arbre[actuel.y][actuel.x].cout_g = arbre[actuel.y][actuel.x].parent->cout_g + calculerDistance(actuel, depart);
		arbre[actuel.y][actuel.x].cout_h = calculerDistance(actuel, arrivee);
		arbre[actuel.y][actuel.x].cout_f = arbre[actuel.y][actuel.x].cout_g + arbre[actuel.y][actuel.x].cout_h;
		for(i = 0; i < 4; i++) {
			prochain.x = vois[i][0];
			prochain.y = vois[i][1];
			if(prochain.x < 0 || prochain.y < 0 || prochain.x >= laby.taille.x || prochain.y >= laby.taille.y) continue;
			if(!separation(laby, actuel, prochain) && visitees[prochain.y][prochain.x] == 0) {
				int dist = calculerDistance(prochain, arrivee);
				if(dist < distanceMini || distanceMini == 0) {
					distanceMini = dist;
					plusProche = i;
				}
				fils++;
			}
		}
		if(fils == 0) {
			actuel.x = file[indice].x;
			actuel.y = file[indice].y;
			indice++;
		}
		else {
			for(i = 0; i < 4; i++) {
				prochain.x = vois[i][0];
				prochain.y = vois[i][1];
				if(prochain.x < 0 || prochain.y < 0 || prochain.x >= laby.taille.x || prochain.y >= laby.taille.y) continue;
				if(!separation(laby, actuel, prochain) && visitees[prochain.y][prochain.x] == 0 && i != plusProche) {
					file[ajout] = prochain;
					if(arbre[prochain.y][prochain.x].parent == NULL) {
						arbre[prochain.y][prochain.x].parent = &(arbre[actuel.y][actuel.x]);
					}
					else if(arbre[prochain.y][prochain.x].parent->cout_f > arbre[actuel.y][actuel.x].cout_f) {
						arbre[prochain.y][prochain.x].parent = &(arbre[actuel.y][actuel.x]);
					}
					ajout++;
					if(ajout % 10 == 0) {
						file = realloc(file, (ajout + 11) * sizeof(coordonnees_t)); // Pour ne pas gaspiller trop de mémoire
					}
				}
			}
			int ppx = vois[plusProche][0], ppy = vois[plusProche][1];
			if(arbre[ppy][ppx].parent == NULL) {
				arbre[ppy][ppx].parent = &(arbre[actuel.y][actuel.x]);
			}
			else if(arbre[ppy][ppx].parent->cout_f > arbre[actuel.y][actuel.x].cout_f) {
				arbre[ppy][ppx].parent = &(arbre[actuel.y][actuel.x]);
			}
			actuel.x = ppx;
			actuel.y = ppy;
		}
	}
	free(visitees);
	free(file);
	visitees = NULL;
	file = NULL;
	actuel.x = laby.taille.x - 1;
	actuel.y = laby.taille.y - 1;
	chemin[0].x = actuel.x;
	chemin[0].y = actuel.y;
	Noeud noeudTmp;
	while(!coordonneesEgales(actuel, depart)) {
		noeudTmp = *(arbre[actuel.y][actuel.x].parent);
		chemin[*taille].x = noeudTmp.x;
		chemin[*taille].y = noeudTmp.y;
		actuel.x = noeudTmp.x;
		actuel.y = noeudTmp.y;
		*taille = *taille + 1;
	}
	free(arbre);
	arbre = NULL;
}

void affichageGraphique(labyrinthe_t laby, int victor, int attente) {
	// Affiche le labyrinthe de façon graphique
	int x, y, i;
	coordonnees_t *chemin;
	int taille = 1;
	int tailleCaseX = LARGEUR / laby.taille.x;
	int tailleCaseY = HAUTEUR / laby.taille.y;
	MLV_draw_filled_rectangle(0, 0, LARGEUR, HAUTEUR, MLV_COLOR_WHITE);
	MLV_draw_line(0, 0, tailleCaseX * laby.taille.x, 0, MLV_COLOR_BLACK);
	MLV_draw_line(tailleCaseX * laby.taille.x - 1, 0, tailleCaseX * laby.taille.x - 1, tailleCaseY * (laby.taille.y - 1), MLV_COLOR_BLACK);
	MLV_draw_line(0, tailleCaseY, 0, tailleCaseY * laby.taille.y, MLV_COLOR_BLACK);
	for(y = 0; y < laby.taille.y; y++) {
		for(x = 0; x < laby.taille.x; x++) {
			char rac[10];
			char rgTexte[10];
			coordonnees_t racine = trouver(laby, x, y);
			int rg = laby.cases[y][x].rang;
			sprintf(rac, "(%d ; %d)", racine.x, racine.y);
			sprintf(rgTexte, "%d", rg);
			/*MLV_draw_adapted_text_box(x * tailleCaseX + 30, y * tailleCaseY + 10, rac, 5,
			MLV_COLOR_BLACK, !(racine.x == x && racine.y == y) ? MLV_COLOR_BLUE : MLV_COLOR_RED, MLV_COLOR_GREY,
			MLV_TEXT_CENTER);
			if(racine.x == x && racine.y == y) {
				MLV_draw_adapted_text_box(x * tailleCaseX + 30, y * tailleCaseY + 50, rgTexte, 5,
			MLV_COLOR_BLACK, (racine.x == x && racine.y == y) ? MLV_COLOR_BLUE : MLV_COLOR_RED, MLV_COLOR_GREY,
			MLV_TEXT_CENTER);
			}*/ // Ces lignes de code permettent juste d'afficher les coordonnées de la racine d'un point
			if(laby.cases[y][x].murEst) {
				MLV_draw_line((x + 1) * tailleCaseX, y * tailleCaseY, (x + 1) * tailleCaseX, (y + 1) * tailleCaseY, MLV_COLOR_BLACK);
			}
			if(laby.cases[y][x].murSud) {
				MLV_draw_line(x * tailleCaseX, (y + 1) * tailleCaseY, (x + 1) * tailleCaseX, (y + 1) * tailleCaseY, MLV_COLOR_BLACK);
			}
		}
	}
	if(victor == 1) {
		chemin = (coordonnees_t *) calloc(laby.taille.x * laby.taille.y, sizeof(coordonnees_t));
		cheminPlusCourt(laby, chemin, &taille);
		chemin = (coordonnees_t *) realloc(chemin, taille * sizeof(coordonnees_t)); // Pour ne pas gaspiller trop de mémoire
		for(i = taille - 1; i > - 1; i--) {
			MLV_draw_filled_ellipse(chemin[i].x * tailleCaseX + tailleCaseX / 2, chemin[i].y * tailleCaseY + tailleCaseY / 2, tailleCaseX / 2, tailleCaseY / 2, MLV_COLOR_BLUE);
			if(attente > 0) {
				MLV_actualise_window();
				MLV_wait_milliseconds(attente);
			}
		}
		free(chemin);
		chemin = NULL;
	}
	MLV_actualise_window();
}

void lier(labyrinthe_t *laby, coordonnees_t point, coordonnees_t liaison) {
	// Lie dans le labyrinthe le point liaison et (x, y)
	coordonnees_t racine = trouver(*laby, 0, 0);
	if(liaison.x > point.x) {
		laby->cases[point.y][point.x].murEst = 0;
	}
	else if(liaison.x < point.x) {
		laby->cases[point.y][liaison.x].murEst = 0;
	}
	else if(liaison.y < point.y) {
		laby->cases[liaison.y][point.x].murSud = 0;
	}
	else {
		laby->cases[point.y][point.x].murSud = 0;
	}
	relierCases(laby, point, trouver(*laby, point.x, point.y), liaison, racine);
}

void construireLabyrinthe(labyrinthe_t *laby, int attente, int mode, int unique, int access, int victor, int optim, Mur *murs) {
    int entree;
    clock_t debut;
    int tailleCaseX = LARGEUR / laby->taille.x;
	int tailleCaseY = HAUTEUR / laby->taille.y;
	int x, y;
	int indiceMurs = 0;
    if(mode == 0) {
		MLV_create_window("Labyrinthe", "Labyrinthe", LARGEUR, HAUTEUR);
		affichageGraphique(*laby, 0, 0);
    }
    else if(mode == 1) {
		if(attente != 0) {
			affichageTexte(*laby);
		}
        if(attente == -1) {
            printf("Appuyer sur entree pour lancer la construction du labyrinthe");
            getchar();
        }
        else if(attente != 0) {
			#ifdef WINDOWS // Le code n'est pas le même selon le système d'exploitation
				Sleep(attente);
			#else
				usleep(attente);
			#endif
        }
    }
    debut = clock();
    while(!memeClasse(*laby, 0, 0, laby->taille.x - 1 , laby->taille.y - 1)) {
        supprimerMur(laby, unique, optim, murs, indiceMurs);
        indiceMurs++;
        if(mode == 0 && attente != 0) {
            affichageGraphique(*laby, 0, 0);
        }
        else if(attente != 0) {
            affichageTexte(*laby);
        }
        if(attente == -1) {
			if(mode == 0) {
				MLV_wait_mouse(&entree, &entree); // On attend que l'utilisateur clique
			}
			else {
				getchar(); // On attend une entrée clavier
			}
        }
        else if(attente != 0) {
			MLV_wait_milliseconds(attente);
        }
    }
    printf("%f mili-secondes pour construire le labyrinthe %s ameliorations\n", (double) (clock() - debut) / 1000.0, optim ? "avec" : "sans");
    if(access == 1) { // On veut relier toutes les cases dans la même classe
		int choix[4]; // On considère choix comme une sorte de liste
		int indice = 0;
		for(y = 0; y < laby->taille.y; y++) {
			for(x = 0; x < laby->taille.x; x++) {
				if(!memeClasse(*laby, x, y, 0, 0)) {
					indice = 0;
					int voisins[4][2] = {{x + 1, y}, {x, y + 1}, {x - 1, y}, {x, y - 1}}; // Les voisins de notre point actuel
					int j;
					for(j = 0; j < 4; j++) {
						if(voisins[j][0] < 0 || voisins[j][1] < 0 || voisins[j][0] >= laby->taille.x || voisins[j][1] >= laby->taille.y) {
							continue;
						}
						if(memeClasse(*laby, voisins[j][0], voisins[j][1], 0, 0)) {
							choix[indice] = j; // On ajoute ce choix à notre "liste"
							indice++;
						}
					}
					coordonnees_t liaison;
					int alea = rand() % indice;
					liaison.x = voisins[choix[alea]][0];
					liaison.y = voisins[choix[alea]][1];
					coordonnees_t point;
					point.x = x;
					point.y = y;
					lier(laby, point, liaison); // On relie les deux cellules pour mettre la cellule point dans la classe principale
					if(mode == 0 && attente != 0) {
						affichageGraphique(*laby, 0, 0);
						MLV_draw_filled_rectangle(x * tailleCaseX, y * tailleCaseY, tailleCaseX, tailleCaseY, MLV_COLOR_YELLOW);
						MLV_draw_filled_rectangle(liaison.x * tailleCaseX, liaison.y * tailleCaseY, tailleCaseX, tailleCaseY, MLV_COLOR_GREEN);
						MLV_actualise_window();
						if(attente == -1) {
							MLV_wait_mouse(&entree, &entree);
						}
						else {
							MLV_wait_milliseconds(attente);
						}
					}
					else if(attente != 0) {
						affichageTexte(*laby);
						if(attente == -1) {
							getchar();
						}
						else if(attente != 0) {
							#ifdef WINDOWS // Le code n'est pas le même selon le système d'exploitation
								Sleep(attente);
							#else
								usleep(attente);
							#endif
						}
					}
				}
			}
		}
	}
    if(mode == 0) {
		affichageGraphique(*laby, victor, attente);
		MLV_wait_mouse(&entree, &entree);
		MLV_free_window();
	}
	else {
		affichageTexte(*laby);
	}
}

void melanger(Mur *tableau, int taille, int tx, int ty) {
	int i, indice = 0, alea, x = 0, y = 0;
	int xtmp, ytmp, murtmp;
	for(y = 0; y < ty; y++) {
		for(x = 0; x < tx; x++) {
			if(x + 1 < tx || y + 1 < ty) {
				tableau[indice].x = x;
				tableau[indice].y = y;
			}
			if(x + 1 < tx && y + 1 < ty) {
				tableau[indice].x = x;
				tableau[indice].y = y;
				tableau[indice].murEst = 1;
				indice++;
				tableau[indice].x = x;
				tableau[indice].y = y;
			}
			else if(x + 1 == tx && y + 1 < ty) {
				tableau[indice].murEst = 1;
			}
			indice++;
		}
	}
	for(i = 0; i < taille; i++) {
		alea = rand() % taille;
		xtmp = tableau[alea].x;
		ytmp = tableau[alea].y;
		murtmp = tableau[alea].murEst;
		tableau[alea].x = tableau[i].x;
		tableau[alea].y = tableau[i].y;
		tableau[alea].murEst = tableau[i].murEst;
		tableau[i].x = xtmp;
		tableau[i].y = ytmp;
		tableau[i].murEst = murtmp;
	}
}

int main(int argc, char **argv)
{
    labyrinthe_t laby;
    coordonnees_t taille;
    Mur *aleatoire;
    int mode = 0;
    int attente = 0;
    unsigned int graine = time(NULL);
    int unique = 0;
    int access = 0;
    int victor = 0;
    int optim = 0;
    taille.x = 6;
    taille.y = 8;
    int arg;
    for(arg = 1; arg < argc; arg++) {
        char *argument = argv[arg];
        if(argument[0] == '-' && argument[1] == '-') {
            argument = argument + 2;
            char *param = strtok(argument, "="); // la fonction split (diviser) en C qui permet de diviser une chaine de caractère selon un délimiteur
            if(strcmp(param, "taille") == 0) {
                param = strtok(NULL, "= x"); // on divise grâce au x
                taille.x = atoi(param);
                param = strtok(NULL, "= x"); // pareil ici
                taille.y = atoi(param);
            }
            else if(strcmp(param, "graine") == 0) {
                param = strtok(NULL, "=");
                graine = atoi(param);
            }
            else if(strcmp(param, "attente") == 0) {
                param = strtok(NULL, "=");
                attente = atoi(param);
            }
            else if(strcmp(param, "unique") == 0) {
                unique = 1;
            }
            else if(strcmp(param, "mode") == 0) {
                param = strtok(NULL, "=");
                if(strcmp(param, "texte") == 0) {
					mode = 1;
				}
			}
			else if(strcmp(param, "acces") == 0) {
				access = 1;
			}
			else if(strcmp(param, "optim") == 0) {
				optim = 1;
			}
        }
        else {
            printf("Argument %d '%s' invalide\n", arg - 1, argument);
        }
    }
    laby.taille = taille;
    srand(graine);
    if(!initialiser(&laby)) {
		printf("Echec de l'initialisation du labyrinthe\n");
	}
	if(optim) {
		aleatoire = (Mur *) calloc(2 * laby.taille.x * laby.taille.y - laby.taille.x - laby.taille.y, sizeof(Mur)); // Qui est le résultat de 2 * (x - 1) * (y - 1) + x + y - 2 où x et y représentent la taille du labyrinthe
		melanger(aleatoire, 2 * laby.taille.x * laby.taille.y - laby.taille.x - laby.taille.y, laby.taille.x, laby.taille.y);
	}
    construireLabyrinthe(&laby, attente, mode, unique, access, victor, optim, aleatoire);
    free(aleatoire);
    aleatoire = NULL;
    if(optim) {
		free(aleatoire);
		aleatoire = NULL;
	}
	printf("La graine était %d.", graine);
    return 0;
}
