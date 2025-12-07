import React from 'react';
import styles from './styles.module.css';

export const Hero: React.FC<{
  title?: string;
  subtitle?: string;
}> = ({ title = 'Crous', subtitle = 'Binary Serialization for Python' }) => (
  <div className={styles.hero}>
    <h1>{title}</h1>
    <p>{subtitle}</p>
  </div>
);

export const Card: React.FC<{
  title: string;
  description: string;
  icon?: string;
}> = ({ title, description, icon }) => (
  <div className={styles.card}>
    {icon && <div className={styles.cardIcon}>{icon}</div>}
    <h3>{title}</h3>
    <p>{description}</p>
  </div>
);

export const CardGrid: React.FC<{
  children: React.ReactNode;
}> = ({ children }) => (
  <div className={styles.cardGrid}>
    {children}
  </div>
);

export const Badge: React.FC<{
  label: string;
  value: string;
  color?: string;
}> = ({ label, value, color = '#0ea5e9' }) => (
  <span
    className={styles.badge}
    style={{
      backgroundColor: color,
      color: '#fff',
      padding: '4px 12px',
      borderRadius: '20px',
      fontSize: '0.875rem',
      fontWeight: '600',
      display: 'inline-block',
      marginRight: '8px',
    }}
  >
    {label}: <strong>{value}</strong>
  </span>
);

export const Feature: React.FC<{
  title: string;
  description: string;
  emoji?: string;
  link?: string;
}> = ({ title, description, emoji = '✨', link = '#' }) => (
  <a href={link} className={styles.featureLink}>
    <div className={styles.feature}>
      <span className={styles.featureEmoji}>{emoji}</span>
      <h4>{title}</h4>
      <p>{description}</p>
    </div>
  </a>
);

export const CodeComparison: React.FC<{
  left: { title: string; code: string };
  right: { title: string; code: string };
}> = ({ left, right }) => (
  <div className={styles.comparison}>
    <div className={styles.comparisonItem}>
      <h4>{left.title}</h4>
      <pre><code>{left.code}</code></pre>
    </div>
    <div className={styles.comparisonItem}>
      <h4>{right.title}</h4>
      <pre><code>{right.code}</code></pre>
    </div>
  </div>
);
