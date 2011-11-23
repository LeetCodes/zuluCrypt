  
#include "includes.h"

int removekey(  int argn , char * device, char * keyType, char * keytoremove  )
{
	StrHandle *p;
	int status = 0 , z = 0 ;
	struct stat st ;	
	char *c ;
	
	if (  stat(  device,&st  ) != 0  ){
		status = 10 ;
		goto out ;
	}
	
	if (  argn == 3  ){
		
		printf( "Enter the passphrase of the key you want to delete: " ) ;
		
		p = get_passphrase(  ) ;
		
		printf( "\n" ) ;		
		
		status = remove_key(  device,StringContent( p ) ) ;
		
		StringDelete(  p  ) ;
		
	}else if (  argn == 5  ){
		
		if(  strcmp( keyType, "-f" ) == 0  ){
			
			if (  stat(  keytoremove,&st  ) != 0  ){
				status =  5 ;
				goto out;
			}
			
			c = (  char *  ) malloc (  sizeof(  char  ) * (  st.st_size + 1  )  ) ;
			
			if(  c == NULL  ){
				status = 7 ;
				goto out ;
			}
			
			*(  c + st.st_size   ) = '\0' ;
			
			z = open(  keytoremove, O_RDONLY  ) ;
			
			read(  z , c , st.st_size  ) ;
			
			close(  z  ) ;		
			
			status = remove_key(  device,c  ) ;
			
			free(  c  ) ;
			
		}else if(  strcmp( keyType, "-p" ) == 0  ) {
			
			status = remove_key(  device,keytoremove  ) ;		
		}
	}else
		status = 6 ;
	
	out:
	switch (  status  ){
		case 0 : printf( "SUCCESS: key removed successfully\n" );
		break ;
		case 1 : printf( "ERROR: device \"%s\" is not a luks device",device ) ;
		break ;
		case 2 : printf( "ERROR: there is no key in the volume that match the presented key\n" ) ;
		break ;
		case 3 : printf( "ERROR: could not open device\n" ) ;
		break ;  
		case 5 : printf( "ERROR: keyfile does not exist\n" ) ;
		break ;
		case 6 : printf( "ERROR: Wrong number of arguments\n" ) ;
		break ;
		case 7 : printf( "ERROR: insuffucient system memory, quiting\n" ) ;
		break ;
		case 10 : printf( "ERROR: device does not exist\n" );
		break ;		
		default :
			;		
	}		
	return status ;	
}

 