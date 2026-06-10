// Get temperature(s) from ALS31313 device(s)
//


#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <argp.h>

#define als31313_VERSION_MAJOR 0
#define als31313_VERSION_MINOR 0

// CLI Arguments
// argp
const char *argp_program_version = "als31313_pi";
const char *argp_program_bug_address = "pjvint@gmail.com";

static char doc[] = "als31313 - simple CLI application to read an ALS31313 3D Hall Effect Sensor device";
static char args_doc[] = "ARG1 [STRING...]";


static struct argp_option options[] = {
	{ "bus", 'b', "BUS", 0, "Bus number" },
	{ "address", 'a', "ADDRESS", 0, "Address (ie 0x40)" },
	{ "Resolution", 'r', "RESOLUTION", 0, "ADC Resolution. 0-3, 0=Max (18bit) 3 = min (12 bit)" },
	{ "filter", 'f', "FILTER", 0, "Filter coeffocient" },
	{ "delay", 'd', "DELAY", 0, "Loop delay (ms) (if not set display once and exit)" },
	{ "quiet", 'q', 0, 0, "Suppress normal output, return temperature" },
	{ "verbose", 'v', "VERBOSITY", 0, "Verbose output" },
	{ "help", 'h', 0, 0, "Show help" },
	{ 0 }
};

/* Used by main to communicate with parse_opt. */
struct arguments
{
	char *args[2];                /* arg1 & arg2 */
	unsigned int bus, address, verbose, resolution, filter, delay, quiet;
};

/* Parse a single option. */
static error_t parse_opt ( int key, char *arg, struct argp_state *state )
{
	/* Get the input argument from argp_parse, which we
	know is a pointer to our arguments structure. */
	struct arguments *arguments = state->input;

	switch ( key )
	{
		case 'b':
			arguments->bus = atoi( arg );
			break;
		case 'a':
			arguments->address = strtoul( arg, NULL, 16 );
			break;
		case 'v':
			arguments->verbose = strtoul( arg, NULL, 10 );
			break;
		case 'r':
			arguments->resolution = atoi( arg );
			break;
		case 'd':
			arguments->delay = atoi( arg ) * 1000;
			break;
		case 'f':
			arguments->filter = atoi( arg );
			break;
		case 'q':
			arguments->quiet = 1;
			break;
		case 'h':
			//print_usage( "als31313" );
			printf("Try --usage\n");
			exit( 0 );
			break;
		case ARGP_KEY_ARG:
			if ( state->arg_num >= 2 )
			{
				argp_usage( state );
			}
			arguments->args[ state->arg_num ] = arg;
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

// TODO: Add syslog ability
void printLog( char *msg, unsigned int verbose, unsigned int level )
{
	if ( level > verbose )
		return;

	fprintf( stderr, "%d: %s\n", level, msg );
	return;
}

int sensorConfig(  unsigned int bus, unsigned int address, unsigned char filterCoefficient, unsigned char config )
{
	int file;
	
	file = initHardware ( bus, address );

	ioctl( file, I2C_SLAVE, address );

	// write customer access code 0x2c413534
	write( file, 0x24, 0x2c413534);

	// Set loop mode (TODO: Make this an option)
	unsigned int value0x27;

	read(file, value0x27, 4);
	value0x27 = (value0x27 & 0xfffffff3) | (0x0 << 2);

	write(file, value0x27, 4);

	return file;	

}

float readHall( int file, unsigned int address )
{
	ioctl( file, I2C_SLAVE, address );


	char cfg[2];
	char reg1[1];

	unsigned int data;

	write(file, 0x28, 1);
	read(file, data, 4);

	printf("0x%08x\n", data);
	// Test getting ID
	char r[1] = {0x20};
	write( file, r, 1 );
	read( file, cfg, 2 );
	printf("\nDeviceID: %02x %02x\n", cfg[0], cfg[1] );

	cfg[0] = 0x05;
	cfg[1] = 0x00;

	write( file, cfg, 2 );

	cfg[0] = 0x06;
	cfg[1] = 0x00;

	//char 
reg1[0] = 0x04;
	write(file, reg1, 1);

}

float readAmbientTemp( int file, unsigned int address )
{
	ioctl( file, I2C_SLAVE, address );

	char cfg[2];

	char reg1[1] = {0x02};
	write(file, reg1, 1);
	char data[2] = {0};
	read( file, data, 2 );

	int lowTemp = data[0] & 0x80;
	float ret;
	if ( lowTemp )
	{
		ret = data[0] * 16 + data[1] / 16 - 4096;
	}
	else
	{
		ret = data[0] * 16 + data[1] * 0.0625;
	}
	printf("ret : %f\n", ret);
	return ret;
}

int main( int argc, char **argv )
{
	struct arguments arguments;

	/* Default values. */
	arguments.bus = 1;
	arguments.address = 0x60;
	arguments.verbose = 0;
	arguments.resolution = 0;
	arguments.delay = 0;
	arguments.filter = 0;
	arguments.quiet = 0;

	/* Parse our arguments; every option seen by parse_opt will
	be reflected in arguments. */
	argp_parse ( &argp, argc, argv, 0, 0, &arguments );

	unsigned char config = arguments.resolution;

	int file = sensorConfig( arguments.bus, arguments.address, arguments.filter, config );

	while ( 1 )
	{
		float temp = readHall( file, arguments.address );

		if ( arguments.quiet != 0 )
		{
			return (int) temp;
		}

		printf("%.2f\n", temp);

		if ( arguments.delay == 0 )
		{
			break;
		}

		usleep ( arguments.delay );
	}

	return 0;
}

int initHardware( unsigned int adpt, unsigned int addr )
{
	int file;
	char bus[11];

	sprintf( bus, "/dev/i2c-%01d", adpt );

	if ( ( file = open( bus, O_RDWR ) ) < 0 )
	{
		char msg[256];
		snprintf( msg, 256, "Error opening %s\n", bus );
		printLog( msg, 1, 1 );
		exit( 1 );
	}

	ioctl( file, I2C_SLAVE, addr );

	return file;
}
