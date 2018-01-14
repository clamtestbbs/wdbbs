#! /usr/bin/perl

# Get Parameters
if ($ENV{'REQUEST_METHOD'} eq "POST") {
   read(STDIN, $buffer, $ENV{'CONTENT_LENGTH'});
} elsif ($ENV{'REQUEST_METHOD'} eq "GET") {
        $buffer=$ENV{'QUERY_STRING'};
}

# Make Pair of Name and Value
@pairs = split(/&/, $buffer);
foreach $pair (@pairs)
{
    ($name, $value) = split(/=/, $pair);

    $value =~ tr/+/ /;
    $value =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/eg;
    $INPUT{$name} = $value;
}

print "Content-type: text/html\n\n";

if ($INPUT{'id'} eq "") 
  { print "½Ð¿é¤J±z¦b WD-BBS ¤Wªº ID\n"; exit;}
elsif ($INPUT{'passwd'} eq "") 
  { print "½Ð¿é¤J±z¦b WD-BBS ¤Wªº±K½X\n"; exit;}
elsif ($INPUT{'board'} eq "") 
  { print "½Ð¿é¤J±z­n¶Kªº¬ÝªO\n"; exit;}
elsif ($INPUT{'subject'} eq "") 
  { print "½Ð¿é¤J¤å³¹ªº¼ÐÃD\n"; exit;}
elsif ($INPUT{'article'} eq "") 
  { print "¤£¯à¶KªÅ¥Õªº¤å³¹­ò!\n"; exit;}
else {
        open(TMP, "> /home/bbs/tmp/mailpost_$INPUT{'id'}");
        print TMP "#name: $INPUT{'id'}\n";
        print TMP "#password: $INPUT{'passwd'}\n";
        print TMP "#board: $INPUT{'board'}\n";
        print TMP "#title: $INPUT{'subject'}\n\n";
        print TMP "$INPUT{'article'}\n\n";
        print TMP "--\n[1;36m¡°Post by [37m$INPUT{'id'} [36mfrom [33m$ENV{'REMOTE_ADDR'}[m\n";
        close(TMP);
	system("/usr/bin/mail bbs < /home/bbs/tmp/mailpost_$INPUT{'id'}");
	system("/bin/rm /home/bbs/tmp/mailpost_$INPUT{'id'}");
        print "<body bgcolor=000000 text=ffff70>";   
        print "¨Ó¦Û $ENV{'REMOTE_ADDR'} ªº $INPUT{id} ±z¦n...<br>";
        print "±zªº¤å³¹¤w¸g°e¥X ... ¦pªG±b¸¹©Î±K½X¿ù»~·|µLªk¶K¤W !<br>";
}
