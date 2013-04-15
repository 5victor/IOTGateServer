    export CLASSPATH=bcprov-jdk15on-148.jar
    CERTSTORE=./mystore.bks
    if [ -a $CERTSTORE ]; then
        rm $CERTSTORE || exit 1
    fi
    keytool \
      -import \
      -v \
      -trustcacerts \
      -alias 0 \
      -file <(openssl x509 -in server.pem) \
      -keystore $CERTSTORE \
      -storetype BKS \
      -provider org.bouncycastle.jce.provider.BouncyCastleProvider \
      -providerpath /usr/share/java/bcprov.jar \
      -storepass 123456

