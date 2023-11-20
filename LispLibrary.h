// ListLibrary.h 

// T-Deck Lisp Editor - Version 2 - 4th November 2023

const char LispLibrary[] PROGMEM = R"lisplibrary(

(defvar *cmds* nil)
(defvar *atomic-cmds* nil)
(defvar *binary-cmds* nil)

(defun %edit (fun)
  (let ((cmd (car *cmds*)))
    (cond
     ((null *cmds*) fun)
     ((eq cmd #\h) (pop *cmds*) (%edit (cons 'highlight (list fun))))
     ((consp cmd) (funcall (cdr (assoc (car cmd) *binary-cmds*)) (cdr cmd) fun))
     ((assoc cmd *atomic-cmds*) (funcall (cdr (assoc cmd *atomic-cmds*)) fun))
     (t fun))))

(defun butlast (lst) (subseq lst 0 (1- (length lst))))

(defun edit (name)
  (let ((fun (eval name))
        cc)
    (setq *cmds* nil)
    (loop
     (write-byte 12)
     (setq cc (append cc (list #\h)))
     (setq *cmds* cc)
     (pprint (%edit fun))
     (setq cc (butlast cc))
     (let ((c (get-key)))
       (cond
         ((eq c #\q) (return name))
         ((eq c #\s) (setq *cmds* cc) (set name (%edit fun)) (return name))
         ((assoc c *binary-cmds*)
          (write-byte 11) (princ c) (princ #\:)
          (setq cc (append cc (list (cons c (read))))))
         ((assoc c *atomic-cmds*) (setq cc (append cc (list c))))
         (t (write-byte 7)))))))

(defun define-atomic-cmd (char name cmd) (push (cons char cmd) *atomic-cmds*))

(defun define-binary-cmd (char name cmd) (push (cons char cmd) *binary-cmds*))

(define-atomic-cmd #\b "back"
  #'(lambda (fun) (pop *cmds*) fun))

(define-atomic-cmd #\d "cdr"
  #'(lambda (fun) (pop *cmds*) (if (atom fun) (%edit fun) (%edit (cons (car fun) (%edit (cdr fun)))))))

(define-atomic-cmd #\a "car"
  #'(lambda (fun) (pop *cmds*) (if (atom fun) (%edit fun) (%edit (cons (%edit (car fun)) (cdr fun))))))

(define-atomic-cmd #\x "delete"
  #'(lambda (fun) (pop *cmds*) (if (atom fun) (%edit fun) (%edit (cdr fun)))))

(define-binary-cmd #\r "replace"
  #'(lambda (val fun) (pop *cmds*) (if (atom fun) (%edit val) (%edit fun))))

(define-binary-cmd #\c "cons"
  #'(lambda (val fun) (pop *cmds*) (%edit (cons val fun))))

(define-binary-cmd #\i "insert"
  #'(lambda (val fun) (pop *cmds*) (%edit (cons val fun))))

(define-binary-cmd #\f "find"
  #'(lambda (val fun)
      (cond
       ((null fun) nil)
       ((equal val fun) (pop *cmds*) (%edit fun))
       ((atom fun) fun)
       (t (cons (%edit (car fun)) (%edit (cdr fun)))))))

(defun load-package (filename)
  (with-sd-card (str filename)
    (loop
     (let (ln (eval (read str)))
       (unless ln (return nothing))))))

(defun save-package (filename lst)
  (with-sd-card (str filename 2)
   (dolist (f lst)
      (symbol-def f str))))

(defun add-to-package (filename list)
   (with-sd-card (str filename 1)
     (dolist (f lst)
         (symbol-def f str))))

(defun lister (filename)
  (with-sd-card (str filename)
    (loop
     (let ((ln (read-line str)))
       (unless ln (return nothing))
       (princ ln)
       (terpri)))))

(defun lisp-server (ipstring)
  (let ((println #'(lambda (x s) (format s "~a~a~%" x #\return))))
    (with-client (s ipstring 8080)
      (print "Listening...")
      (loop
       (unless (= 0 (available s))
         (let* ((line (read-line s))
                (result (ignore-errors (eval (read-from-string line)))))
           (print line)
           (println (if (eq result nothing) "Error!" result) s)))
       (delay 1000)))))

(defun bat nil (+ 0.2 (* (analogread 4) (/ 6.6 4095.0))))

)lisplibrary";
