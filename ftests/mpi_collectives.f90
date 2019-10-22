module test_collectives
use mpi
implicit none

contains
subroutine check_collectives(size, rank)
implicit none
integer, intent(in) :: size, rank
integer :: i, ierror
double precision, dimension(size) :: local, recv
double precision :: err

do i = 1, size
        local(i) = rank
end do

call MPI_BARRIER(MPI_COMM_WORLD, ierror)
call MPI_ALLREDUCE(local, recv, size, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD, ierror)

do i = 1, size
        err = recv(i) - ((size*(size-1))/2)
        if(err > 1e-14) then
                call exit(1)
        endif
end do
end subroutine
end module test_collectives

program mpi_collectives
use test_collectives
implicit none
integer :: rank, size, ierror

call MPI_INIT(ierror)
call MPI_COMM_SIZE(MPI_COMM_WORLD, size, ierror)
call MPI_COMM_RANK(MPI_COMM_WORLD, rank, ierror)

call check_collectives(size, rank)
call MPI_FINALIZE(ierror)

end

