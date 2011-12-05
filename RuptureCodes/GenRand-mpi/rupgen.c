#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <mpi.h>
#include <unistd.h>
#include <dirent.h>
#include "structure.h"
#include "func_mpi.h"
#include "genslip.h"
#include "function.h"

/* Constants */
#define MAX_STR_ARGV 512
#define MAX_NUM_FILES 50000
#define RUP_FILE_EXT ".txt"
#define RUP_FILE_DELIM "_"


/* Rupture magnitude comparator, descending order */
int rupcomp(const void *p1, const void *p2)
{
  rg_rfile_t *r1;
  rg_rfile_t *r2;

  r1 = (rg_rfile_t *)p1;
  r2 = (rg_rfile_t *)p2;

  if (r1->mag < r2->mag) {
    return(1);
  } else if (r1-> mag == r2->mag) {
    return(0);
  } else {
    return(-1);
  }
}


/* Parse rupture file names */
int parserup(const char *path, const char *fname, 
	     int *src, int *rup, float *mag)
{
  char tmpstr[MAX_FILENAME];
  char *tok;
  FILE *fp;

  *mag = -1.0;
  *src = -1;
  *rup = -1;

  memset(tmpstr, 0, MAX_FILENAME);
  strncpy(tmpstr, fname, strstr(fname, RUP_FILE_EXT) - fname);

  /* Find source and rupture */
  tok = strtok(tmpstr, RUP_FILE_DELIM);
  if (tok != NULL) {
    *src = atoi(tok);
    tok = strtok(NULL, RUP_FILE_DELIM);
    if (tok != NULL) {
      *rup = atoi(tok);
    } else {
      return(1);
    }
  } else {
    return(1);
  }

  /* Find magnitude */
  sprintf(tmpstr, "%s/%d/%d/%s", path, *src, *rup, fname);
  fp = fopen(tmpstr, "r");
  if (fp == NULL) {
    return(1);
  }
  while (!feof(fp)) {
    if (fgets(tmpstr, MAX_FILENAME, fp) != NULL) {
      if (strstr(tmpstr, "Magnitude = ") != NULL) {
	/* Parse magnitude */
	sscanf(tmpstr, "%*s %*s %f", mag);
	break;
      }
    }
  }
  fclose(fp);

  if ((*mag < 0.0) || (*src < 0) || (*rup < 0)) {
    return(1);
  }

  return(0);
}


/* Get number of rupture files */
int getnumrups(const char *path, int *numrup)
{
  FILE *fp;
  char buf[MAX_FILENAME];
  char index[MAX_FILENAME];

  *numrup = 0;
  sprintf(index, "%s/index.list", path);
  fp = fopen(index, "r");
  if (fp == NULL) {
    return(1);
  }

  while (!feof(fp)) {
    if (fgets(buf, MAX_FILENAME, fp) != NULL) {
      if (strstr(buf, RUP_FILE_EXT) != NULL) {
	(*numrup)++;
      }
    }
  }
  fclose(fp);
  return(0);
}


/* Get list of rupture files */
int getrups(const char *path, rg_rfile_t *rups, int maxrup, int *numrup)
{
  FILE *fp;
  char buf[MAX_FILENAME];
  char index[MAX_FILENAME];
  int i;

  *numrup = 0;
  sprintf(index, "%s/index.list", path);
  fp = fopen(index, "r");
  if (fp == NULL) {
    return(1);
  }

  while (!feof(fp)) {
    if (fgets(buf, MAX_FILENAME, fp) != NULL) {
      if (sscanf(buf, "%d %d %f %s", &(rups[*numrup].src),
		 &(rups[*numrup].rup),
		 &(rups[*numrup].mag),
		 rups[*numrup].filename) == 4) {
	if (rups[*numrup].filename[strlen(rups[*numrup].filename) - 1] 
	    == '\n') {
	  rups[*numrup].filename[strlen(rups[*numrup].filename) - 1] = '\0';
	}
	if ((strlen(rups[*numrup].filename) == 0) || 
	    (rups[*numrup].src < 0) || 
	    (rups[*numrup].rup < 0) || 
	    (rups[*numrup].mag <= 0.0)) {
	  fprintf(stderr, "Invalid rupture file rec: %s\n", buf);
	  return(1);
	}
	rups[(*numrup)].index = *numrup;
	rups[(*numrup)].stats.numslip = 0;
	rups[(*numrup)].stats.numhypo = 0;
	(*numrup)++;
      }
    }
  }
  fclose(fp); 

  /* Sort ruptures by magnitude in descending order */
  qsort(rups, *numrup, sizeof(rg_rfile_t), rupcomp);

  /* Reassign indices after sort */
  for (i = 0; i < *numrup; i++) {
    rups[i].index = i;
  }
  return(0);
}


/* Run Rob Graves GenSlip rupture generator */
int run_genslip(char *infile, char *outfile, char *logfile,
		rg_stats_t *stats)
{
  int j, rgargc;
  char **rgargv = NULL;

  /* Create pseudo-program args */
  rgargc = 4;
  rgargv = malloc(rgargc * sizeof(char*));
  for (j = 0; j < rgargc; j++) {
    rgargv[j] = malloc(MAX_STR_ARGV);
  }
  sprintf(rgargv[0], "%s", "rupgen");
  sprintf(rgargv[1], "infile=%s", infile);
  sprintf(rgargv[2], "outfile=%s", outfile);
  sprintf(rgargv[3], "logfile=%s", logfile);

  /* Run rupture generator */
  genslip(rgargc, rgargv, stats);
  
  /* Free pseudo-program args */
  rgargc = 4;
  for (j = 0; j < rgargc; j++) {
    free(rgargv[j]);
  }

  free(rgargv);
  return(0);
}


int main(int argc, char **argv)
{
  /* MPI stuff and distributed computation variables */
  int myid, nproc, pnlen;
  char procname[128];
  MPI_Status status;

  /* MPI Rupture info vars */
  MPI_Datatype MPI_RUP_T;
  int num_fields_rup;

  char rupdir[MAX_FILENAME];
  char logdir[MAX_FILENAME];
  char infile[MAX_FILENAME];
  char outfile[MAX_FILENAME];
  char logfile[MAX_FILENAME];
  rg_rfile_t *rups = NULL;
  int i, n, numrup, numvar;
  rg_rfile_t rup;
  int currup = 0;
  int done = 0;

  FILE *sfp = NULL;

  /* Init MPI */
  mpi_init(&argc, &argv, &nproc, &myid, procname, &pnlen);

  /* Register new mesh data types */
  if (mpi_register_rupinfo(&MPI_RUP_T, &num_fields_rup) != 0) {
      fprintf(stderr, "[%d] Failed to register data type\n", myid);
      return(1);
  }

  if (myid == 0) {
    if (argc != 3) {
      printf("usage: %s rupdir logdir", argv[0]);
      return(0);
    }

    if (nproc < 2) {
      fprintf(stderr, "[%d] nproc must be at least 2\n", myid);
      return(1);
    }
  }

  strcpy(rupdir, argv[1]);
  strcpy(logdir, argv[2]);
  numrup = 0;

  if (myid == 0) {

    printf("[%d] rupdir=%s\n", myid, rupdir);
    printf("[%d] logdir=%s\n", myid, logdir);
    fflush(stdout);

    /* Get number of rupture files */
    if (getnumrups(rupdir, &numrup) != 0) {
      fprintf(stderr, "[%d] Failed to count rupture files\n", myid);
      return(1);
    }

    if (numrup == 0) {
      fprintf(stderr, "[%d] No rupture files to process\n", myid);
      return(1);
    } else {
      printf("[%d] Retrieved %d rupture files\n", myid, numrup);
    }

    rups = malloc(numrup * sizeof(rg_rfile_t));
    if (rups == NULL) {
      fprintf(stderr, "[%d] Failed to allocate rupture file buffer\n",
	      myid);
      return(1);
    }
    memset(rups, 0, sizeof(rg_rfile_t)*numrup);

    /* Get rupture files */
    if (getrups(rupdir, rups, numrup, &n) != 0) {
      fprintf(stderr, "[%d] Failed to find rupture files\n", myid);
      return(1);
    }
  }

  /* Main loop */
  memset(&rup, 0, sizeof(rg_rfile_t));
  if (myid == 0) {
    /* Dispatch rupture files to worker pool */
    currup = 0;
    while (currup < numrup) {
      //printf("[%d] currup=%d, numrup=%d\n", myid, currup, numrup);
      /* Blocking receive on worker pool */
      MPI_Recv(&rup, 1, MPI_RUP_T, MPI_ANY_SOURCE, MPI_ANY_TAG, 
	       MPI_COMM_WORLD, &status);

      /* Update rupture record if necessary */
      if (strlen(rup.filename) > 0) {
	if ((rup.index < 0) || (rup.src < 0) || (rup.rup < 0) || 
	    (rup.stats.numslip <= 0) || (rup.stats.numhypo <= 0)) {
	  fprintf(stderr, "[%d] Worker %d returned bad data for rupture %d\n", 
		  myid, status.MPI_SOURCE, rup.index);
	  return(1);
	}
	memcpy(&(rups[rup.index]), &rup, sizeof(rg_rfile_t));
      }

      /* Send next available rupture to worker */
      if ((strlen(rups[currup].filename) == 0) || 
	  (rups[currup].src < 0) || (rups[currup].rup < 0)) {
	fprintf(stderr, "[%d] Detected bad rupture at index %d\n", 
		myid, currup);
	return(1);
      }
      memcpy(&rup, &(rups[currup]), sizeof(rg_rfile_t));
      MPI_Send(&rup, 1, MPI_RUP_T, status.MPI_SOURCE, 
	       0, MPI_COMM_WORLD);
      if ((currup % 100 == 0) && (currup > 0)) {
	printf("[%d] Dispatched %d ruptures\n", myid, currup);
      }
      currup++;
    }

    /* Determine number of workers still in progress */
    int numworking = 0;
    for (i = 0; i < numrup; i++) {
      if (rups[i].stats.numslip == 0) {
	numworking++;
      }
    }
    printf("[%d] %d workers still working\n", myid, numworking);
    fflush(stdout);

    /* Send stop message to all workers */
    printf("[%d] All ruptures dispatched, stopping workers\n", myid);
    for (i = 1; i < nproc; i++) {
      /* Blocking receive on worker pool */
      MPI_Recv(&rup, 1, MPI_RUP_T, MPI_ANY_SOURCE, MPI_ANY_TAG, 
	       MPI_COMM_WORLD, &status);

      /* Update rupture record if necessary */
      if (strlen(rup.filename) > 0) {
	if ((rup.index < 0) || (rup.src < 0) || (rup.rup < 0) || 
	    (rup.stats.numslip <= 0) || (rup.stats.numhypo <= 0)) {
	  fprintf(stderr, "[%d] Worker %d returned bad data for rupture %d\n", 
		  myid, status.MPI_SOURCE, rup.index);
	  return(1);
	}
	memcpy(&(rups[rup.index]), &rup, sizeof(rg_rfile_t));
      }

      /* Send stop work to worker */
      memset(&rup, 0, sizeof(rg_rfile_t));
      MPI_Send(&(rup), 1, MPI_RUP_T, status.MPI_SOURCE, 
	       0, MPI_COMM_WORLD);
      if ((i % 100 == 0) || ((nproc - i) < 20))  {
	printf("[%d] Stopped %d workers\n", myid, i);
      }
    }
  } else {
    while (!done) {
      /* Send work request to rank 0 */
      MPI_Send(&(rup), 1, MPI_RUP_T, 0, 0, MPI_COMM_WORLD);

      /* Blocking receive on next rupture */
      memset(&rup, 0, sizeof(rg_rfile_t));
      MPI_Recv(&rup, 1, MPI_RUP_T, 0, MPI_ANY_TAG, 
	       MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      /* Check to see if we are done */
      if (strlen(rup.filename) == 0) {
	done = 1;
      } else {
	sprintf(infile, "%s%s", rupdir, rup.filename);
	if (!file_exists(infile)) {
	  fprintf(stderr, "[%d] Rupture file %s not found\n",
		  myid, infile);
	  return(1);
	}

	//printf("[%d] Processing %s, index %d, source %d, rupture %d\n", 
	//       myid, infile, rup.index, rup.src, rup.rup);

	/* Create log directory */
	sprintf(logfile, "%s/%d", logdir, rup.src);
	mkdir(logfile, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	sprintf(logfile, "%s/%d/rupgen_%d_%d.log", logdir, rup.src,
		rup.src, rup.rup);

	/* Generate the rupture variations */
	sprintf(outfile, "%s%s.variation", rupdir, rup.filename);
	//printf("[%d] Processing file %s\n", myid, infile);
	if (run_genslip(infile, outfile, logfile, &(rup.stats)) != 0) {
	  fprintf(stderr, "[%d] Failed to run rupture generator\n", myid);
	  return(1);
	}
      }
    }
  }

  /* Print summary statistics */
  if (myid == 0) {
    sprintf(outfile, "%s/variations.list", rupdir);
    sfp = fopen(outfile, "w");
    if (sfp == NULL) {
      fprintf(stderr, "[%d] Failed to open statistics file %s\n", 
	      myid, outfile);
      return(1);
    }

    numvar = 0;
    for (i = 0; i < numrup; i++) {
      numvar += (rups[i].stats.numslip * rups[i].stats.numhypo);
      fprintf(sfp, "%d %d %d %d\n", rups[i].src, rups[i].rup,
	      rups[i].stats.numslip, rups[i].stats.numhypo);
    }

    fclose(sfp);

    printf("[%d] Ruptures: %d\n", myid, numrup);
    printf("[%d] Rupture Variations: %d\n", myid, numvar);
    fflush(stdout);
    free(rups);
  }

  /* Final sync */
  mpi_barrier();
  mpi_final("MPI Done");

  return(0);
}
