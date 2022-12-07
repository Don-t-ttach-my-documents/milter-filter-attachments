#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include <libmilter/mfapi.h>
#include <curl/curl.h>

#include "parsing.h"

#ifndef bool
# define bool	int
# define TRUE	1
# define FALSE	0
#endif /* ! bool */


struct mlfiPriv
{
	char	*mlfi_fname;
	char	*mlfi_connectfrom;
	char	*mlfi_helofrom;
	FILE	*mlfi_fp;
	unsigned char *newBody;
	int newBodyLength;
};

#define MLFIPRIV	((struct mlfiPriv *) smfi_getpriv(ctx))

extern sfsistat		mlfi_cleanup(SMFICTX *, bool);

/* recipients to add and reject (set with -a and -r options) */
char *add = NULL;
char *reject = NULL;

sfsistat
mlfi_connect(ctx, hostname, hostaddr)
	 SMFICTX *ctx;
	 char *hostname;
	 _SOCK_ADDR *hostaddr;
{

	struct mlfiPriv *priv;
	char *ident;

	/* allocate some private memory */
	priv = malloc(sizeof *priv);
	if (priv == NULL)
	{
		/* can't accept this message right now */
		return SMFIS_TEMPFAIL;
	}
	memset(priv, '\0', sizeof *priv);

	/* save the private data */
	smfi_setpriv(ctx, priv);
	//priv->newBody = NULL;
	ident = smfi_getsymval(ctx, "_");
	if (ident == NULL)
		ident = "???";
	if ((priv->mlfi_connectfrom = strdup(ident)) == NULL)
	{
		(void) mlfi_cleanup(ctx, FALSE);
		return SMFIS_TEMPFAIL;
	}

	/* continue processing */
	return SMFIS_CONTINUE;
}

sfsistat
mlfi_helo(ctx, helohost)
	 SMFICTX *ctx;
	 char *helohost;
{

	size_t len;
	char *tls;
	char *buf;
	struct mlfiPriv *priv = MLFIPRIV;

	tls = smfi_getsymval(ctx, "{tls_version}");
	if (tls == NULL)
		tls = "No TLS";
	if (helohost == NULL)
		helohost = "???";
	len = strlen(tls) + strlen(helohost) + 3;
	if ((buf = (char*) malloc(len)) == NULL)
	{
		(void) mlfi_cleanup(ctx, FALSE);
		return SMFIS_TEMPFAIL;
	}
	snprintf(buf, len, "%s, %s", helohost, tls);
	if (priv->mlfi_helofrom != NULL)
		free(priv->mlfi_helofrom);
	priv->mlfi_helofrom = buf;

	/* continue processing */
	return SMFIS_CONTINUE;
}

sfsistat
mlfi_envfrom(ctx, argv)
	 SMFICTX *ctx;
	 char **argv;
{

	int fd = -1;
	int argc = 0;
	struct mlfiPriv *priv = MLFIPRIV;
	char *mailaddr = smfi_getsymval(ctx, "{mail_addr}");

	/* open a file to store this message */
	if ((priv->mlfi_fname = strdup("/tmp/msg.XXXXXX")) == NULL)
	{
		(void) mlfi_cleanup(ctx, FALSE);
		return SMFIS_TEMPFAIL;
	}

	if ((fd = mkstemp(priv->mlfi_fname)) == -1)
	{
		(void) mlfi_cleanup(ctx, FALSE);
		return SMFIS_TEMPFAIL;
	}

	if ((priv->mlfi_fp = fdopen(fd, "w+")) == NULL)
	{
		(void) close(fd);
		(void) mlfi_cleanup(ctx, FALSE);
		return SMFIS_TEMPFAIL;
	}

	/* count the arguments */
	while (*argv++ != NULL)
		++argc;

	/* log the connection information we stored earlier: */
	if (fprintf(priv->mlfi_fp, "Connect from %s (%s)\n\n",
		    priv->mlfi_helofrom, priv->mlfi_connectfrom) == EOF)
	{
		(void) mlfi_cleanup(ctx, FALSE);
		return SMFIS_TEMPFAIL;
	}
	/* log the sender */
	if (fprintf(priv->mlfi_fp, "FROM %s (%d argument%s)\n",
		    mailaddr ? mailaddr : "???", argc,
		    (argc == 1) ? "" : "s") == EOF)
	{
		(void) mlfi_cleanup(ctx, FALSE);
		return SMFIS_TEMPFAIL;
	}

	/* continue processing */
	return SMFIS_CONTINUE;
}

sfsistat
mlfi_envrcpt(ctx, argv)
	 SMFICTX *ctx;
	 char **argv;
{

	struct mlfiPriv *priv = MLFIPRIV;
	char *rcptaddr = smfi_getsymval(ctx, "{rcpt_addr}");
	int argc = 0;

	/* count the arguments */
	while (*argv++ != NULL)
		++argc;

	if (fprintf(priv->mlfi_fp, "RCPT %s (%d argument%s)\n",
		    rcptaddr ? rcptaddr : "???", argc,
		    (argc == 1) ? "" : "s") == EOF)
	{
		(void) mlfi_cleanup(ctx, FALSE);
		return SMFIS_TEMPFAIL;
	}

	/* continue processing */
	return SMFIS_CONTINUE;
}

sfsistat
mlfi_header(ctx, headerf, headerv)
	 SMFICTX *ctx;
	 char *headerf;
	 unsigned char *headerv;
{
	/* write the header to the log file */
	if (fprintf(MLFIPRIV->mlfi_fp, "%s: %s\n", headerf, headerv) == EOF)
	{
		(void) mlfi_cleanup(ctx, FALSE);
		return SMFIS_TEMPFAIL;
	}
	
	//TODO Vérifier le header si notre filtre a déjà fait son travail

	/* continue processing */
	return SMFIS_CONTINUE;
}

sfsistat
mlfi_eoh(ctx)
	 SMFICTX *ctx;
{
	/* output the blank line between the header and the body */
	if (fprintf(MLFIPRIV->mlfi_fp, "\n") == EOF)
	{
		(void) mlfi_cleanup(ctx, FALSE);
		return SMFIS_TEMPFAIL;
	}

	/* continue processing */
	return SMFIS_CONTINUE;
}

sfsistat
mlfi_body(ctx, bodyp, bodylen)
	 SMFICTX *ctx;
	 unsigned char *bodyp;
	 size_t bodylen;
{
        struct mlfiPriv *priv = MLFIPRIV;

	/* output body block to log file */
	if (fwrite(bodyp, bodylen, 1, priv->mlfi_fp) != 1)
	{
		/* write failed */
		fprintf(stderr, "Couldn't write file %s: %s\n",
			priv->mlfi_fname, strerror(errno));
		(void) mlfi_cleanup(ctx, FALSE);
		return SMFIS_TEMPFAIL;
	}


	/* continue processing */
	// Get the new body
	struct MemoryStruct res = sendBodyToParsing(bodyp, bodylen);
	priv->newBody = res.memory;
	//To insert new body length
	priv->newBodyLength = strlen(priv->newBody);

	//------DEBUG only
	// fprintf(stderr, "\n------------------------\n");
	// fprintf(stderr, "%s", bodyp);
	// fprintf(stderr, "\n----------------------\n");
	//----------------
	return SMFIS_CONTINUE;
}

sfsistat
mlfi_eom(ctx)
	 SMFICTX *ctx;
{
	bool ok = TRUE;
	struct mlfiPriv *priv = MLFIPRIV;
	fprintf(stderr, "%s", priv->newBody);
	if (smfi_replacebody(ctx, priv->newBody, priv->newBodyLength)==MI_FAILURE){
		fprintf(stderr, "Replace body failed");
		ok = FALSE;
	}
	else 
		fprintf(stderr, "Replace body succeed");

	//TODO add header

	return mlfi_cleanup(ctx, ok);
}

sfsistat
mlfi_abort(ctx)
	 SMFICTX *ctx;
{
	return mlfi_cleanup(ctx, FALSE);
}

sfsistat
mlfi_cleanup(ctx, ok)
	 SMFICTX *ctx;
	 bool ok;
{
	sfsistat rstat = SMFIS_CONTINUE;
	struct mlfiPriv *priv = MLFIPRIV;
	char *p;
	char host[512];
	char hbuf[1024];

	if (priv == NULL)
		return rstat;

	/* close the archive file */
	if (priv->mlfi_fp != NULL && fclose(priv->mlfi_fp) == EOF)
	{
		/* failed; we have to wait until later */
		fprintf(stderr, "Couldn't close archive file %s: %s\n",
			priv->mlfi_fname, strerror(errno));
		rstat = SMFIS_TEMPFAIL;
		(void) unlink(priv->mlfi_fname);
	}
	else if (ok)
	{
		/* add a header to the message announcing our presence */
		if (gethostname(host, sizeof host) < 0)
			snprintf(host, sizeof host, "localhost");
		p = strrchr(priv->mlfi_fname, '/');
		if (p == NULL)
			p = priv->mlfi_fname;
		else
			p++;
		snprintf(hbuf, sizeof hbuf, "%s@%s", p, host);
	}
	else
	{
		/* message was aborted -- delete the archive file */
		fprintf(stderr, "Message aborted.  Removing %s\n",
			priv->mlfi_fname);
		rstat = SMFIS_TEMPFAIL;
		(void) unlink(priv->mlfi_fname);
	}

	/* release private memory */
	if (priv->mlfi_fname != NULL)
		free(priv->mlfi_fname);
	if (priv->newBody != NULL)
		free(priv->newBody);
	/* return status */
	return rstat;
}

sfsistat
mlfi_close(ctx)
	 SMFICTX *ctx;
{
	struct mlfiPriv *priv = MLFIPRIV;

	if (priv == NULL)
		return SMFIS_CONTINUE;
	if (priv->mlfi_connectfrom != NULL)
		free(priv->mlfi_connectfrom);
	if (priv->mlfi_helofrom != NULL)
		free(priv->mlfi_helofrom);
	free(priv);
	smfi_setpriv(ctx, NULL);
	return SMFIS_CONTINUE;
}

sfsistat
mlfi_unknown(ctx, cmd)
	SMFICTX *ctx;
	char *cmd;
{
	return SMFIS_CONTINUE;
}

sfsistat
mlfi_data(ctx)
	SMFICTX *ctx;
{
	return SMFIS_CONTINUE;
}

sfsistat
mlfi_negotiate(ctx, f0, f1, f2, f3, pf0, pf1, pf2, pf3)
	SMFICTX *ctx;
	unsigned long f0;
	unsigned long f1;
	unsigned long f2;
	unsigned long f3;
	unsigned long *pf0;
	unsigned long *pf1;
	unsigned long *pf2;
	unsigned long *pf3;
{
	return SMFIS_ALL_OPTS;
}

struct smfiDesc smfilter =
{
	"AttachmentServerFilter",	/* filter name */
	SMFI_VERSION,	/* version code -- do not change */
	SMFIF_ADDHDRS|SMFIF_ADDRCPT,
			/* flags */
	mlfi_connect,	/* connection info filter */
	mlfi_helo,	/* SMTP HELO command filter */
	mlfi_envfrom,	/* envelope sender filter */
	mlfi_envrcpt,	/* envelope recipient filter */
	mlfi_header,	/* header filter */
	mlfi_eoh,	/* end of header */
	mlfi_body,	/* body block filter */
	mlfi_eom,	/* end of message */
	mlfi_abort,	/* message aborted */
	mlfi_close,	/* connection cleanup */
	mlfi_unknown,	/* unknown SMTP commands */
	mlfi_data,	/* DATA command */
	mlfi_negotiate	/* Once, at the start of each SMTP connection */
};

static void
usage(prog)
	char *prog;
{
	fprintf(stderr,
		"Usage: %s -p socket-addr [-t timeout] [-r reject-addr] [-a add-addr]\n",
		prog);
}

int
main(argc, argv)
	 int argc;
	 char **argv;
{
	bool setconn = FALSE;
	int c;
	const char *args = "p:t:r:a:h";
	extern char *optarg;

	/* Process command line options */

	while ((c = getopt(argc, argv, args)) != -1)
	{

		switch (c)
		{
		  case 'p':
			if (optarg == NULL || *optarg == '\0')
			{
				(void) fprintf(stderr, "Illegal conn: %s\n",
					       optarg);
				exit(EX_USAGE);
			}
			if (smfi_setconn(optarg) == MI_FAILURE)
			{
				(void) fprintf(stderr,
					       "smfi_setconn failed\n");
				exit(EX_SOFTWARE);
			}

			/*
			**  If we're using a local socket, make sure it
			**  doesn't already exist.  Don't ever run this
			**  code as root!!
			*/

			if (strncasecmp(optarg, "unix:", 5) == 0)
				unlink(optarg + 5);
			else if (strncasecmp(optarg, "local:", 6) == 0)
				unlink(optarg + 6);
			setconn = TRUE;
			break;

		  case 't':
			if (optarg == NULL || *optarg == '\0')
			{
				(void) fprintf(stderr, "Illegal timeout: %s\n",
					       optarg);
				exit(EX_USAGE);
			}
			if (smfi_settimeout(atoi(optarg)) == MI_FAILURE)
			{
				(void) fprintf(stderr,
					       "smfi_settimeout failed\n");
				exit(EX_SOFTWARE);
			}
			break;

		  case 'r':
			if (optarg == NULL)
			{
				(void) fprintf(stderr,
					       "Illegal reject rcpt: %s\n",
					       optarg);
				exit(EX_USAGE);
			}
			reject = optarg;
			break;

		  case 'a':
			if (optarg == NULL)
			{
				(void) fprintf(stderr,
					       "Illegal add rcpt: %s\n",
					       optarg);
				exit(EX_USAGE);
			}
			add = optarg;
			smfilter.xxfi_flags |= SMFIF_ADDRCPT;
			break;

		  case 'h':
		  default:
			usage(argv[0]);
			exit(EX_USAGE);
		}
	}

	if (!setconn)
	{
		fprintf(stderr, "%s: Missing required -p argument\n", argv[0]);
		usage(argv[0]);
		exit(EX_USAGE);
	}
	if (smfi_register(smfilter) == MI_FAILURE)
	{
		fprintf(stderr, "smfi_register failed\n");
		exit(EX_UNAVAILABLE);
	}

	return smfi_main();
}
