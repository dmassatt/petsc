FasdUAS 1.101.10   ��   ��    k             l     ��  ��    i c http://stackoverflow.com/questions/6022009/adding-files-via-applescript-to-an-xcode-project-target     � 	 	 �   h t t p : / / s t a c k o v e r f l o w . c o m / q u e s t i o n s / 6 0 2 2 0 0 9 / a d d i n g - f i l e s - v i a - a p p l e s c r i p t - t o - a n - x c o d e - p r o j e c t - t a r g e t   
  
 l     ��  ��    J D This does not work, apparently Applescript is broken with xcode :-(     �   �   T h i s   d o e s   n o t   w o r k ,   a p p a r e n t l y   A p p l e s c r i p t   i s   b r o k e n   w i t h   x c o d e   : - (   ��  i         I     �� ��
�� .aevtoappnull  �   � ****  o      ���� 0 argv  ��    k    0       l     ��������  ��  ��        l     ��  ��    A ; Get the folder containing items to be added to the project     �   v   G e t   t h e   f o l d e r   c o n t a i n i n g   i t e m s   t o   b e   a d d e d   t o   t h e   p r o j e c t      O     6    k    5      !   r     " # " I   	�� $��
�� .earsffdralis        afdr $  f    ��   # o      ���� 0 
thisscript 
thisScript !  % & % r     ' ( ' e     ) ) n     * + * m    ��
�� 
cfol + o    ���� 0 
thisscript 
thisScript ( o      ����  0 projectfolder0 projectFolder0 &  , - , r     . / . e     0 0 c     1 2 1 n     3 4 3 m    ��
�� 
cfol 4 o    ���� 0 
thisscript 
thisScript 2 m    ��
�� 
TEXT / o      ���� 0 projectfolder projectFolder -  5 6 5 r    ! 7 8 7 e     9 9 n     : ; : m    ��
�� 
cfol ; o    ����  0 projectfolder0 projectFolder0 8 o      ����  0 projectfolder1 projectFolder1 6  < = < r   " ) > ? > e   " ' @ @ c   " ' A B A n   " % C D C m   # %��
�� 
cfol D o   " #����  0 projectfolder1 projectFolder1 B m   % &��
�� 
TEXT ? o      ����  0 projectfolder2 projectFolder2 =  E F E r   * / G H G b   * - I J I o   * +����  0 projectfolder2 projectFolder2 J m   + , K K � L L  a r c h - i p h o n e H o      ���� 0 sourcefolder sourceFolder F  M�� M r   0 5 N O N b   0 3 P Q P o   0 1���� 0 sourcefolder sourceFolder Q m   1 2 R R � S S  : x c o d e - l i n k s : O o      ���� 0 sourcefolder sourceFolder��    m      T T�                                                                                  MACS  alis    t  Macintosh HD               ��c�H+   #i�
Finder.app                                                      %I���B        ����  	                CoreServices    ���      �͒     #i� #i� #i�  6Macintosh HD:System: Library: CoreServices: Finder.app   
 F i n d e r . a p p    M a c i n t o s h   H D  &System/Library/CoreServices/Finder.app  / ��     U V U l  7 7��������  ��  ��   V  W X W l  7 7�� Y Z��   Y 4 . Get all the files that will be added to Xcode    Z � [ [ \   G e t   a l l   t h e   f i l e s   t h a t   w i l l   b e   a d d e d   t o   X c o d e X  \ ] \ O   7 G ^ _ ^ r   ; F ` a ` l  ; B b���� b n   ; B c d c 1   @ B��
�� 
pnam d n   ; @ e f e 2   > @��
�� 
ditm f l  ; > g���� g c   ; > h i h o   ; <���� 0 sourcefolder sourceFolder i m   < =��
�� 
alis��  ��  ��  ��   a o      ����  0 filestoaddlist filesToAddList _ m   7 8 j j�                                                                                  sevs  alis    �  Macintosh HD               ��c�H+   #i�System Events.app                                               &����y        ����  	                CoreServices    ���      ���     #i� #i� #i�  =Macintosh HD:System: Library: CoreServices: System Events.app   $  S y s t e m   E v e n t s . a p p    M a c i n t o s h   H D  -System/Library/CoreServices/System Events.app   / ��   ]  k l k l  H H��������  ��  ��   l  m n m l  H. o p q o O   H. r s r k   N- t t  u v u l  N N�� w x��   w / ) Open the project using posix-style paths    x � y y R   O p e n   t h e   p r o j e c t   u s i n g   p o s i x - s t y l e   p a t h s v  z { z I  N [�� |��
�� .aevtodocnull  �    alis | l  N W }���� } b   N W ~  ~ l  N S ����� � n   N S � � � 1   O S��
�� 
psxp � o   N O���� 0 projectfolder projectFolder��  ��    m   S V � � � � �  P E T S c . x c o d e p r o j��  ��  ��   {  � � � l  \ \��������  ��  ��   �  � � � l  \ \�� � ���   � R L Give Xcode some time to open the project before we start giving it commands    � � � � �   G i v e   X c o d e   s o m e   t i m e   t o   o p e n   t h e   p r o j e c t   b e f o r e   w e   s t a r t   g i v i n g   i t   c o m m a n d s �  � � � I  \ c�� ���
�� .sysodelanull��� ��� nmbr � m   \ _���� ��   �  � � � l  d d��������  ��  ��   �  � � � r   d p � � � 4   d l�� �
�� 
proj � m   h k � � � � � 
 P E T S c � o      ���� 0 testproject testProject �  � � � l  q+ � � � � O   q+ � � � k   w* � �  � � � r   w � � � � 4   w �� �
�� 
grup � m   { ~ � � � � �  S o u r c e s � o      ���� 0 sourcegroup sourceGroup �  � � � l  �( � � � � O   �( � � � k   �' � �  � � � l  � ��� � ���   � ) # Iterate over all files in the list    � � � � F   I t e r a t e   o v e r   a l l   f i l e s   i n   t h e   l i s t �  � � � X   �% ��� � � k   �  � �  � � � l  � ���������  ��  ��   �  � � � r   � � � � � l  � � ����� � n   � � � � � 1   � ���
�� 
pcnt � o   � ����� 0 i  ��  ��   � o      ���� 0 filename fileName �  � � � l  � ���������  ��  ��   �  � � � l  � ��� � ���   � Z T Get the file path using Unix-style pathing, since that is the kind that Xcode needs    � � � � �   G e t   t h e   f i l e   p a t h   u s i n g   U n i x - s t y l e   p a t h i n g ,   s i n c e   t h a t   i s   t h e   k i n d   t h a t   X c o d e   n e e d s �  � � � r   � � � � � b   � � � � � l  � � ����� � n   � � � � � 1   � ���
�� 
psxp � o   � ����� 0 sourcefolder sourceFolder��  ��   � o   � ����� 0 filename fileName � o      ���� 0 filepath filePath �  � � � l  � ���������  ��  ��   �  � � � l  � ��� � ���   � * $ Don't add duplicate file references    � � � � H   D o n ' t   a d d   d u p l i c a t e   f i l e   r e f e r e n c e s �  ��� � Z   �  � ����� � H   � � � � E  � � � � � n   � � � � � 1   � ���
�� 
ppth � n  � � � � � 2  � ���
�� 
filr � o   � ����� 0 sourcegroup sourceGroup � o   � ����� 0 filepath filePath � k   � � �  � � � l  � ��� � ���   � . ( Add a new file reference to the project    � � � � P   A d d   a   n e w   f i l e   r e f e r e n c e   t o   t h e   p r o j e c t �  � � � r   � � � � � I  � ����� �
�� .corecrel****      � null��   � �� � �
�� 
kocl � m   � ���
�� 
filr � �� ���
�� 
prdt � K   � � � � �� � �
�� 
pnam � o   � ����� 0 filename fileName � �� � �
�� 
abph � o   � ����� 0 filepath filePath � �� � �
�� 
rfty � m   � ���
�� reftasrt � �� � �
�� 
ppth � o   � ����� 0 filepath filePath � �� ���
�� 
flen � m   � ���
�� fencmosr��  ��   � o      ���� 0 fileref fileRef �  � � � l   ��������  ��  ��   �  � � � l   �� � ��   � 1 + Add the file reference to the build target     � V   A d d   t h e   f i l e   r e f e r e n c e   t o   t h e   b u i l d   t a r g e t �  r    n    4 ��
�� 
tarR m  ����  o   �� 0 testproject testProject o      �~�~ 0 firsttarget firstTarget 	
	 I �}
�} .coreadd null���     obj  o  �|�| 0 fileref fileRef �{�z
�{ 
insh o  �y�y 0 testproject testProject�z  
 �x l �w�v�u�w  �v  �u  �x  ��  ��  ��  �� 0 i   � o   � ��t�t  0 filestoaddlist filesToAddList � �s l &&�r�q�p�r  �q  �p  �s   � o   � ��o�o 0 sourcegroup sourceGroup �   end group tell    � �    e n d   g r o u p   t e l l � �n l ))�m�l�k�m  �l  �k  �n   � o   q t�j�j 0 testproject testProject �   end project tell    � � "   e n d   p r o j e c t   t e l l � �i l ,,�h�g�f�h  �g  �f  �i   s m   H K�                                                                                      @ alis    H  Macintosh HD               ��c�H+   #i�	Xcode.app                                                       	�E=        ����  	                Applications    ���      ���     #i�  $Macintosh HD:Applications: Xcode.app   	 X c o d e . a p p    M a c i n t o s h   H D  Applications/Xcode.app  / ��   p   end app tell    q �    e n d   a p p   t e l l n �e l //�d�c�b�d  �c  �b  �e  ��       �a�a   �`
�` .aevtoappnull  �   � **** �_ �^�]�\
�_ .aevtoappnull  �   � ****�^ 0 argv  �]   �[�Z�[ 0 argv  �Z 0 i   3 T�Y�X�W�V�U�T�S�R K�Q R j�P�O�N�M�L ��K�J�I�H ��G�F ��E�D�C�B�A�@�?�>�=�<�;�:�9�8�7�6�5�4�3�2�1�0�/
�Y .earsffdralis        afdr�X 0 
thisscript 
thisScript
�W 
cfol�V  0 projectfolder0 projectFolder0
�U 
TEXT�T 0 projectfolder projectFolder�S  0 projectfolder1 projectFolder1�R  0 projectfolder2 projectFolder2�Q 0 sourcefolder sourceFolder
�P 
alis
�O 
ditm
�N 
pnam�M  0 filestoaddlist filesToAddList
�L 
psxp
�K .aevtodocnull  �    alis�J 
�I .sysodelanull��� ��� nmbr
�H 
proj�G 0 testproject testProject
�F 
grup�E 0 sourcegroup sourceGroup
�D 
kocl
�C 
cobj
�B .corecnte****       ****
�A 
pcnt�@ 0 filename fileName�? 0 filepath filePath
�> 
filr
�= 
ppth
�< 
prdt
�; 
abph
�: 
rfty
�9 reftasrt
�8 
flen
�7 fencmosr�6 
�5 
�4 .corecrel****      � null�3 0 fileref fileRef
�2 
tarR�1 0 firsttarget firstTarget
�0 
insh
�/ .coreadd null���     obj �\1� 3)j E�O��,EE�O��,�&E�O��,EE�O��,�&E�O��%E�O��%E�UO� ��&�-�,E` UOa  ��a ,a %j Oa j O*a a /E` O_  �*a a /E` O_  � �_ [a a l kh �a  ,E` !O�a ,_ !%E` "O_ a #-a $,_ " V*a a #a %�_ !a &_ "a 'a (a $_ "a )a *a +a , -E` .O_ a /k/E` 0O_ .a 1_ l 2OPY h[OY�zOPUOPUOPUOPascr  ��ޭ