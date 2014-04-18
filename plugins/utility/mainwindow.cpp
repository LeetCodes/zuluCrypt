/*
 *
 *  Copyright (c) 2012
 *  name : mhogo mchungu
 *  email: mhogomchungu@gmail.com
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

#include <sys/types.h>
#include <unistd.h>

MainWindow::MainWindow( QWidget * parent ) : QWidget( parent ),m_ui( new Ui::MainWindow ),m_findExecutable( 0 )
{
	m_ui->setupUi( this ) ;
	this->setFixedSize( this->size() ) ;

	m_ui->lineEditKey->setEchoMode( QLineEdit::Password ) ;

	this->setWindowIcon( QIcon( QString( ":/default.png" ) ) ) ;
	//m_ui->pbKeyFile->setIcon( QIcon( QString( ":/default.png" ) ) );

	connect( m_ui->pbCancel,SIGNAL( clicked() ),this,SLOT( pbCancel() ) ) ;
	connect( m_ui->pbOpen,SIGNAL( clicked() ),this,SLOT( pbOpen() ) ) ;
	connect( m_ui->pbKeyFile,SIGNAL( clicked() ),this,SLOT( pbKeyFile() ) ) ;

	m_ui->lineEditKey->setFocus() ;

	m_working = false ;

	m_requireKey = false ;
	m_requireKeyFile = true ;

	QAction * ac = new QAction( this ) ;
	QList<QKeySequence> keys ;
	keys.append( Qt::Key_Enter );
	keys.append( Qt::Key_Return );
	ac->setShortcuts( keys ) ;
	connect( ac,SIGNAL( triggered() ),this,SLOT( defaultButton() ) ) ;
	this->addAction( ac ) ;
}

void MainWindow::Show()
{
	this->setWindowTitle( tr( "%1 key module" ).arg( m_appName ) ) ;
	this->show() ;
}

void MainWindow::setButtonIcon( const QString& icon )
{
	QString x( ":/" + icon ) ;
	this->setWindowIcon( QIcon( x ) ) ;
	m_ui->pbKeyFile->setIcon( QIcon( x ) ) ;
}

void MainWindow::setRequireKey( bool k )
{
	m_requireKey = k ;
}

void MainWindow::setRequireKeyFile( bool k )
{
	m_requireKeyFile = k ;
}

void MainWindow::defaultButton()
{
	if( m_ui->pbCancel->hasFocus() ){
		this->pbCancel() ;
	}else{
		this->pbOpen() ;
	}
}

void MainWindow::setToken( const QString& token )
{
	m_token = token ;
}

void MainWindow::setApplicationName( const QString& appName )
{
	m_appName = appName ;
}

void MainWindow::setkeyLabel( const QString& keyLabel )
{
	m_ui->label_2->setText( keyLabel ) ;
}

void MainWindow::setkeyFileLabel( const QString& keyFileLabel )
{
	m_ui->label->setText( keyFileLabel ) ;
}

void MainWindow::setKeyFunction( function_t function )
{
	m_function = function ;
}

void MainWindow::SetFocus()
{
	if( m_ui->lineEditKey->text().isEmpty() ){
		m_ui->lineEditKey->setFocus() ;
	}else if( m_ui->lineEditKeyFile->text().isEmpty() ){
		m_ui->lineEditKeyFile->setFocus() ;
	}else{
		m_ui->pbOpen->setFocus() ;
	}
}

void MainWindow::pbCancel()
{
	if( m_working ){
		DialogMsg msg( this ) ;
		int st = msg.ShowUIYesNoDefaultNo( tr( "warning"),
						   tr( "are you sure you want to terminate this operation prematurely?" ) ) ;

		if( st == QMessageBox::Yes ){
			this->enableAlll() ;
			m_working = false ;
			this->cancelled() ;
		}
	}else{
		this->cancelled() ;
		this->Exit( 1 ) ;
	}
}

void MainWindow::cancelled()
{
	getKey::cancel( m_token ) ;
	this->Exit( 1 ) ;
}

void MainWindow::Exit( int st )
{
	char * e = m_key.data() ;

	memset( e,'\0',m_key.size() ) ;

	QCoreApplication::exit( st ) ;
}

void MainWindow::setfindExeFunction( std::function<const QString&( QVector<QString>& )> f )
{
	m_findExecutable = f ;
}

void MainWindow::setExe( const QVector<QString>& exe )
{
	m_exe = exe ;
}

void MainWindow::pbOpen()
{
	DialogMsg msg( this ) ;

	m_key = m_ui->lineEditKey->text().toLatin1() ;
	if( m_requireKey ){
		if( m_key.isEmpty() ){
			return msg.ShowUIOK( tr( "ERROR" ),tr( "key field is empty" ) ) ;
		}
	}

	m_path = m_ui->lineEditKeyFile->text() ;

	m_path.replace( "file://","" ) ;

	if( m_requireKeyFile ){
		if( m_path.isEmpty() ){
			return msg.ShowUIOK( tr( "ERROR" ),tr( "path to %1 keyfile is empty" ).arg( m_appName ) ) ;
		}
		if( !QFile::exists( m_path ) ){
			return msg.ShowUIOK( tr( "ERROR" ),tr( "invalid path to %1 keyfile" ).arg( m_appName ) ) ;
		}
	}

	if( m_findExecutable == 0 ){
		m_findExecutable = []( QVector<QString>& exe ){
			if( exe.isEmpty() ){
				return QString() ;
			}

			QString e ;

			for( auto& it : exe ){
				auto _not_found = [&]( const char * path ){
					e = path + it ;
					bool r = QFile::exists( e ) ;
					if( r ){
						it = e ;
					}
					return r == false ;
				} ;

				if( _not_found( "/usr/local/bin/" ) ){
					if( _not_found( "/usr/bin/" ) ){
						if( _not_found( "/usr/sbin/" ) ){
							return it ;
						}
					}
				}
			}
			return QString() ;
		} ;
	}

	/*
	 * pass in a copy since we modify it in place wrong strings show up on the GUI on error
	 */
	m_exe_1 = m_exe ;
	QString e = m_findExecutable( m_exe_1 ) ;
	if( !e.isEmpty() ){
		return msg.ShowUIOK( tr( "ERROR" ),
				     tr( "could not find \"%1\" executable in \"/usr/local\",\"/usr/bin\" and \"/usr/sbin\"" ).arg( e ) ) ;
	}

	this->disableAll() ;
	m_working = true ;

	this->key() ;
}

void MainWindow::key()
{
	getKey * g = new getKey( m_token ) ;
	connect( g,SIGNAL( done( int ) ),this,SLOT( done( int ) ) ) ;
	g->setOptions( m_exe_1,m_key,m_path,m_function ) ;
	g->start() ;
}

void MainWindow::done( int status )
{
	getKey::status s = getKey::status( status ) ;

	if( s == getKey::cancelled ){
		this->Exit( 1 ) ;
	}else if( s == getKey::complete ){
		this->Exit( 0 ) ;
	}else if( s == getKey::wrongKey ){
		DialogMsg msg( this ) ;
		m_working = false ;
		msg.ShowUIOK( tr( "ERROR" ),tr("could not decrypt the %1 keyfile,wrong key?" ).arg( m_appName ) ) ;
		this->enableAlll() ;
		m_ui->lineEditKey->setFocus() ;
		this->setWindowTitle( tr( "%1 key module" ).arg( m_appName ) ) ;
	}else{
		/*
		 * we cant get here
		 */
	}

}

void MainWindow::pbKeyFile()
{
	QString Z = QFileDialog::getOpenFileName( this,tr( "select a key file" ),QDir::homePath() ) ;

	if( !Z.isEmpty() ){
		m_ui->lineEditKeyFile->setText( Z ) ;
	}
	this->SetFocus() ;
}

void MainWindow::closeEvent( QCloseEvent * e )
{
	e->ignore() ;
	this->pbCancel() ;
}

void MainWindow::disableAll()
{
	m_ui->label->setEnabled( false ) ;
	m_ui->label_2->setEnabled( false ) ;
	m_ui->lineEditKey->setEnabled( false ) ;
	m_ui->lineEditKeyFile->setEnabled( false ) ;
	m_ui->pbKeyFile->setEnabled( false ) ;
	m_ui->pbOpen->setEnabled( false ) ;
	m_ui->pbCancel->setEnabled( false ) ;
}

void MainWindow::enableAlll()
{
	m_ui->label->setEnabled( true ) ;
	m_ui->label_2->setEnabled( true ) ;
	m_ui->lineEditKey->setEnabled( true ) ;
	m_ui->lineEditKeyFile->setEnabled( true ) ;
	m_ui->pbKeyFile->setEnabled( true ) ;
	m_ui->pbOpen->setEnabled( true ) ;
	m_ui->pbCancel->setEnabled( true ) ;
}

MainWindow::~MainWindow()
{
	delete m_ui ;
}