#!/usr/bin/perl -w
#####
# (C) Kay Schulz
# Licensed under the same terms as Mailfilter, GPL v2 or later.
# Use at your own risk!
# Calculates the percentage of the filters' success
# Needs runit.sh, deleted.sh and examined.sh
#####

$deleted = `deleted.sh`;
$examined = `examined.sh`;
printf ("%2.2f%%  (%d) of all mails (%d) deleted\n", $deleted*100/$examined, $deleted, $examined);
print "The top mailfilters are:\n";
@teste = `runit.sh`;
foreach(@teste) 
{
	$zahl = substr ($_, 0, 4);
	$rest = substr ($_, 22);
	if ($zahl > 9)
	{	
		$prozent = $zahl * 100 / $deleted;
		printf ("% 6.2f%% for %s", $prozent, $rest);
	}
}
