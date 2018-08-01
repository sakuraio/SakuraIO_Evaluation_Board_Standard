# Maintenance

## Sync with github.com

### Setup hg-git

```bash
pip install mercurial hg-git
```

#### ~/.hgrc

```
[extensions]
hggit =
```

### mbed.org to github.com

```bash
hg clone https://developer.mbed.org/teams/SAKURA-Internet/code/SakuraIO_Evaluation_Board_Standard/ SakuraIO_Evaluation_Board_Standard-sync
cd SakuraIO_Evaluation_Board_Standard
hg push git@github.com:sakuraio/SakuraIO_Evaluation_Board_Standard.git
```

### github.com to mbed.org

```bash
hg pull git@github.com:sakuraio/SakuraIO_Evaluation_Board_Standard.git
hg outgoing https://developer.mbed.org/teams/SAKURA-Internet/code/SakuraIO_Evaluation_Board_Standard/ # check diff
hg push https://developer.mbed.org/teams/SAKURA-Internet/code/SakuraIO_Evaluation_Board_Standard/
```
