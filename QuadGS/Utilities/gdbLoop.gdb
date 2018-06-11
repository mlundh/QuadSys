set $n = 100
while $n-- > 0
  printf "starting program\n"
  run
  if $_siginfo
    printf "Received signal %d, stopping\n", $_siginfo.si_signo
    loop_break
  else
    printf "program exited\n"
  end
end