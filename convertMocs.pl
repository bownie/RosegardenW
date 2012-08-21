my @FILES=`dir /b /s *.cpp`;
my $swap = 0;
my $tmpFile = "tmp.file";

for my $cur ( @FILES )
{
  chomp($cur);
  $swap = 0;
  
  print $cur."\n";
  
  open RO, $cur or die "Cannot open ".$cur."\n";
  open WO, "> ".$tmpFile;
  
  while ( <RO> )
  {
    if ( m/#include\s+"\w+.moc"/ )
    {
      print "Fixing MOC\n";
      $swap = 1;
      s/#include\s+"(\w+).moc"/#include "moc_$1.cpp"/g;
    }
    
    print WO $_;

  }
  
  close RO;
  close WO;
  
  if ( $swap == 1)
  {
    print "Copying ".$cur."\n";
    system ("copy ".$tmpFile." ".$cur);
  }
  
  
}
