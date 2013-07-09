rucio-admin account add usera
rucio-admin account add userb
rucio-admin account add userc
rucio-admin account add userd
rucio-admin account add usere
rucio-admin account add userf
rucio-admin scope add --account usera --scope usera
rucio-admin scope add --account userb --scope userb
rucio-admin scope add --account userc --scope userc
rucio-admin scope add --account userd --scope userd
rucio-admin scope add --account usere --scope usere
rucio-admin scope add --account userf --scope userf
rucio-admin rse add LOCATIONX
rucio-admin rse add LOCATIONY
rucio-admin rse add LOCATIONZ
rucio add-replicas --lfns usera:file_1 usera:file_2 usera:file_3 --rses LOCATIONX LOCATIONY LOCATIONZ --checksums ad:12345678 ad:12345678 ad:12345678 --bytes 987654321 987654321 987654321
rucio add-replicas --lfns userb:file_1 userb:file_2 userb:file_3 --rses LOCATIONX LOCATIONY LOCATIONZ --checksums ad:12345678 ad:12345678 ad:12345678 --bytes 987654321 987654321 987654321
rucio add-replicas --lfns userc:file_1 userc:file_2 userc:file_3 --rses LOCATIONX LOCATIONY LOCATIONZ --checksums ad:12345678 ad:12345678 ad:12345678 --bytes 987654321 987654321 987654321
rucio add-replicas --lfns userd:file_1 userd:file_2 userd:file_3 --rses LOCATIONX LOCATIONY LOCATIONZ --checksums ad:12345678 ad:12345678 ad:12345678 --bytes 987654321 987654321 987654321
rucio add-replicas --lfns usere:file_1 usere:file_2 usere:file_3 --rses LOCATIONX LOCATIONY LOCATIONZ --checksums ad:12345678 ad:12345678 ad:12345678 --bytes 987654321 987654321 987654321
rucio add-replicas --lfns userf:file_1 userf:file_2 userf:file_3 --rses LOCATIONX LOCATIONY LOCATIONZ --checksums ad:12345678 ad:12345678 ad:12345678 --bytes 987654321 987654321 987654321
rucio add-replicas --lfns usera:file_1 usera:file_2 usera:file_3 --rses LOCATIONY LOCATIONZ LOCATIONX --checksums ad:12345678 ad:12345678 ad:12345678 --bytes 987654321 987654321 987654321
rucio add-replicas --lfns userb:file_1 userb:file_2 userb:file_3 --rses LOCATIONY LOCATIONZ LOCATIONX --checksums ad:12345678 ad:12345678 ad:12345678 --bytes 987654321 987654321 987654321
rucio add-replicas --lfns userc:file_1 userc:file_2 userc:file_3 --rses LOCATIONY LOCATIONZ LOCATIONX --checksums ad:12345678 ad:12345678 ad:12345678 --bytes 987654321 987654321 987654321
rucio add-replicas --lfns userd:file_1 userd:file_2 userd:file_3 --rses LOCATIONY LOCATIONZ LOCATIONX --checksums ad:12345678 ad:12345678 ad:12345678 --bytes 987654321 987654321 987654321
rucio add-replicas --lfns usere:file_1 usere:file_2 usere:file_3 --rses LOCATIONY LOCATIONZ LOCATIONX --checksums ad:12345678 ad:12345678 ad:12345678 --bytes 987654321 987654321 987654321
rucio add-replicas --lfns userf:file_1 userf:file_2 userf:file_3 --rses LOCATIONY LOCATIONZ LOCATIONX --checksums ad:12345678 ad:12345678 ad:12345678 --bytes 987654321 987654321 987654321
rucio add-dataset usera:dataset_a
rucio add-dataset userb:dataset_b
rucio add-dataset userc:dataset_c
rucio add-dataset userd:dataset_d
rucio add-dataset usere:dataset_e
rucio add-dataset userf:dataset_f
rucio attach --to usera:dataset_a usera:file_1 usera:file_2
rucio attach --to userb:dataset_b userb:file_1 userb:file_2
rucio attach --to userc:dataset_c userc:file_1 userc:file_2
rucio attach --to userd:dataset_d userd:file_1 userd:file_2
rucio attach --to usere:dataset_e usere:file_1 usere:file_2
rucio attach --to userf:dataset_f userf:file_1 userf:file_2
rucio add-container usera:container_a
rucio add-container userb:container_b
rucio add-container userc:container_c
rucio add-container userd:container_d
rucio add-container usere:container_e
rucio add-container userf:container_f
rucio attach --to usera:container_a usera:dataset_a userb:dataset_b
rucio attach --to userb:container_b userb:dataset_b userc:dataset_c
rucio attach --to userc:container_c userc:dataset_c userd:dataset_d
rucio attach --to userd:container_d userd:dataset_d usere:dataset_e
rucio attach --to usere:container_e usere:dataset_e userf:dataset_f
rucio attach --to userf:container_f userf:dataset_f usera:dataset_a
rucio add-container usera:container_g
rucio add-container userb:container_h
rucio add-container userc:container_i
rucio attach --to usera:container_g userf:container_f
rucio attach --to userb:container_h usere:container_e
rucio attach --to userc:container_i userd:container_d
