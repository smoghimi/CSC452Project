/*
 * Simple Spawn test.
 */

#include <usloss.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <libuser.h>
#include <stdio.h>

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start3(char *arg)
{

   USLOSS_Console("start3(): started, and immediately return'ing a -3\n");

   return -3;

} /* start3 */
